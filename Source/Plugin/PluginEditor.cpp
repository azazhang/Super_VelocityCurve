#include "PluginEditor.h"
#include <juce_gui_basics/juce_gui_basics.h>

SuperVelocityCurveAudioProcessorEditor::SuperVelocityCurveAudioProcessorEditor (SuperVelocityCurveAudioProcessor& p)
    : AudioProcessorEditor (p),
      audioProcessor (p)
{
    setResizable (true, true);
    setResizeLimits (900, 640, 1600, 1000);
    setSize (1100, 720);

    titleLabel.setFont (svc::ui::Theme::titleFont());
    subtitleLabel.setFont (svc::ui::Theme::smallFont());
    subtitleLabel.setColour (juce::Label::textColourId, juce::Colour (svc::ui::Theme::textSecondary));

    for (auto* c : { &titleLabel, &subtitleLabel, &profileLabel, &outputModeLabel, &presetLabel, &liveHitsLabel })
        addAndMakeVisible (c);

    for (auto* c : { &profileBox, &outputModeBox, &curvePresetBox })
        addAndMakeVisible (c);

    profileNameEditor.setText ("My Profile");
    profileNameEditor.setFont (svc::ui::Theme::bodyFont());
    addAndMakeVisible (profileNameEditor);

    for (auto* b : { &saveProfileButton, &duplicateProfileButton, &deleteProfileButton,
                     &importButton, &exportButton, &resetCurveButton })
        addAndMakeVisible (b);

    outputModeBox.addItemList ({ "Auto (match input)", "MIDI 1.0 (7-bit)", "MIDI 2.0 (high-res)" }, 1);
    curvePresetBox.addItemList ({ "Linear", "Soft (boost ghosts)", "Hard (tame accents)", "S-Curve",
                                  "Exponential", "Logarithmic" }, 1);

    addAndMakeVisible (padGrid);
    addAndMakeVisible (curveEditor);
    addAndMakeVisible (padInspector);

    outputModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        audioProcessor.getApvts(), "outputMode", outputModeBox);

    padGrid.onPadSelected = [this] (int index) { onPadSelected (index); };

    curveEditor.onPadChanged = [this] (const svc::ProfilePad& pad) { updateSelectedPadFromUI (pad); };

    padInspector.onPadChanged = [this] (int index, const svc::ProfilePad& pad)
    {
        juce::ignoreUnused (index);
        updateSelectedPadFromUI (pad);
    };

    profileBox.onChange = [this] { onProfileSelected(); };

    curvePresetBox.onChange = [this]
    {
        curveEditor.applyPreset (static_cast<svc::CurvePreset> (curvePresetBox.getSelectedItemIndex()));
    };

    resetCurveButton.onClick = [this] { curveEditor.resetCurve(); };

    saveProfileButton.onClick = [this]
    {
        const auto name = profileNameEditor.getText().trim();
        if (audioProcessor.getProfileStore().saveActiveAsUserProfile (name))
        {
            rebuildProfileList();
            applyProfileToEngine();
            showStatus ("Saved profile: " + name);
        }
        else
        {
            showStatus ("Enter a profile name to save.", true);
        }
    };

    duplicateProfileButton.onClick = [this]
    {
        if (audioProcessor.getProfileStore().duplicateActiveAsUserProfile ({ }))
        {
            rebuildProfileList();
            showStatus ("Profile duplicated.");
        }
    };

    deleteProfileButton.onClick = [this]
    {
        auto& store = audioProcessor.getProfileStore();
        if (store.getActiveEntryType() != svc::ProfileEntryType::userProfile)
        {
            showStatus ("Only user profiles can be deleted.", true);
            return;
        }

        if (store.deleteUserProfile (store.getActiveEntryIndex()))
        {
            rebuildProfileList();
            onPadSelected (0);
            applyProfileToEngine();
            showStatus ("Profile deleted.");
        }
    };

    importButton.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser> ("Import profile", juce::File{}, "*.xml;*.svcp");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this, chooser] (const juce::FileChooser& fc)
                              {
                                  const auto file = fc.getResult();
                                  if (file.existsAsFile()
                                      && audioProcessor.getProfileStore().importProfileFromFile (file))
                                  {
                                      rebuildProfileList();
                                      onPadSelected (0);
                                      applyProfileToEngine();
                                      showStatus ("Imported " + file.getFileName());
                                  }
                                  else if (file != juce::File())
                                  {
                                      showStatus ("Import failed.", true);
                                  }
                              });
    };

    exportButton.onClick = [this]
    {
        auto chooser = std::make_shared<juce::FileChooser> ("Export profile", juce::File{}, "*.xml");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
                              [this, chooser] (const juce::FileChooser& fc)
                              {
                                  auto file = fc.getResult();
                                  if (file == juce::File())
                                      return;

                                  if (! file.hasFileExtension (".xml"))
                                      file = file.withFileExtension (".xml");

                                  if (audioProcessor.getProfileStore().exportActiveProfileToFile (file))
                                      showStatus ("Exported to " + file.getFileName());
                                  else
                                      showStatus ("Export failed.", true);
                              });
    };

    audioProcessor.getProfileStore().onProfileChanged = [this]
    {
        rebuildProfileList();
        refreshPadUI();
    };

    rebuildProfileList();
    onPadSelected (0);
    startTimerHz (30);
}

void SuperVelocityCurveAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (svc::ui::Theme::background));

    auto header = getLocalBounds().removeFromTop (56).reduced (16, 10);
    g.setColour (juce::Colour (svc::ui::Theme::textPrimary));
    g.setFont (svc::ui::Theme::titleFont());
    g.drawText (titleLabel.getText(), header.removeFromTop (26), juce::Justification::centredLeft);
    g.setFont (svc::ui::Theme::smallFont());
    g.setColour (juce::Colour (svc::ui::Theme::textSecondary));
    g.drawText (subtitleLabel.getText(), header, juce::Justification::centredLeft);

    if (statusMessage.isNotEmpty())
    {
        g.setColour (statusIsError ? juce::Colours::salmon : juce::Colour (svc::ui::Theme::success));
        g.drawText (statusMessage, getLocalBounds().removeFromBottom (22).reduced (16, 0), juce::Justification::centredLeft);
    }
}

void SuperVelocityCurveAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (12).withTrimmedTop (56).withTrimmedBottom (statusMessage.isEmpty() ? 0 : 22);

    auto toolbar = bounds.removeFromTop (120);
    auto col = toolbar.removeFromLeft (toolbar.getWidth() / 2).reduced (4);
    profileLabel.setBounds (col.removeFromTop (16));
    profileBox.setBounds (col.removeFromTop (24));
    col.removeFromTop (4);
    profileNameEditor.setBounds (col.removeFromTop (24));
    col.removeFromTop (4);
    auto profileButtons = col.removeFromTop (24);
    const int btnW = profileButtons.getWidth() / 5;
    saveProfileButton.setBounds (profileButtons.removeFromLeft (btnW).reduced (1));
    duplicateProfileButton.setBounds (profileButtons.removeFromLeft (btnW).reduced (1));
    deleteProfileButton.setBounds (profileButtons.removeFromLeft (btnW).reduced (1));
    importButton.setBounds (profileButtons.removeFromLeft (btnW).reduced (1));
    exportButton.setBounds (profileButtons.reduced (1));

    col = toolbar.reduced (4);
    auto topRow = col.removeFromTop (44);
    auto outCol = topRow.removeFromLeft (topRow.getWidth() / 2).reduced (0, 0);
    outputModeLabel.setBounds (outCol.removeFromTop (16));
    outputModeBox.setBounds (outCol);

    auto presetCol = topRow.reduced (0, 0);
    presetLabel.setBounds (presetCol.removeFromTop (16));
    auto presetRow = presetCol.removeFromTop (24);
    curvePresetBox.setBounds (presetRow.removeFromLeft (presetRow.getWidth() - 90).reduced (0, 0));
    resetCurveButton.setBounds (presetRow.reduced (1));

    liveHitsLabel.setBounds (col.removeFromTop (40));

    auto left = bounds.removeFromLeft (juce::jmax (320, bounds.getWidth() / 2)).reduced (4);
    padGrid.setBounds (left);

    auto right = bounds.reduced (4);
    auto inspector = right.removeFromBottom (220);
    padInspector.setBounds (inspector);
    curveEditor.setBounds (right);
}

void SuperVelocityCurveAudioProcessorEditor::rebuildProfileList()
{
    profileBox.clear();
    const auto entries = audioProcessor.getProfileStore().getProfileList();

    int selectedId = 1;
    int id = 1;
    for (const auto& entry : entries)
    {
        profileBox.addItem (entry.displayName, id);

        if (entry.type == audioProcessor.getProfileStore().getActiveEntryType()
            && entry.index == audioProcessor.getProfileStore().getActiveEntryIndex())
            selectedId = id;

        ++id;
    }

    profileBox.setSelectedId (selectedId, juce::dontSendNotification);
}

void SuperVelocityCurveAudioProcessorEditor::onProfileSelected()
{
    const auto id = profileBox.getSelectedId();
    if (id <= 0)
        return;

    const auto entries = audioProcessor.getProfileStore().getProfileList();
    const auto index = id - 1;
    if (index < 0 || index >= static_cast<int> (entries.size()))
        return;

    const auto& entry = entries[static_cast<size_t> (index)];
    if (entry.type == svc::ProfileEntryType::factoryTemplate)
        audioProcessor.getProfileStore().loadFactoryTemplate (entry.index);
    else
        audioProcessor.getProfileStore().loadUserProfile (entry.index);

    onPadSelected (0);
    applyProfileToEngine();
}

void SuperVelocityCurveAudioProcessorEditor::onPadSelected (int padIndex)
{
    selectedPadIndex = padIndex;
    padGrid.setSelectedPadIndex (padIndex);
    refreshPadUI();
}

void SuperVelocityCurveAudioProcessorEditor::refreshPadUI()
{
    const auto& profile = audioProcessor.getProfileStore().getActiveProfile();
    padGrid.setProfile (profile);

    if (selectedPadIndex >= 0 && selectedPadIndex < static_cast<int> (profile.getPads().size()))
    {
        const auto& pad = profile.getPads()[static_cast<size_t> (selectedPadIndex)];
        curveEditor.setPad (pad);
        padInspector.setPad (pad, selectedPadIndex);
    }
}

void SuperVelocityCurveAudioProcessorEditor::updateSelectedPadFromUI (const svc::ProfilePad& pad)
{
    auto& profile = audioProcessor.getProfileStore().getActiveProfile();
    if (selectedPadIndex < 0 || selectedPadIndex >= static_cast<int> (profile.getPads().size()))
        return;

    profile.getPads()[static_cast<size_t> (selectedPadIndex)] = pad;
    audioProcessor.getProfileStore().syncActiveUserProfileFromEdits();
    applyProfileToEngine();
    padInspector.setPad (pad, selectedPadIndex);
}

void SuperVelocityCurveAudioProcessorEditor::applyProfileToEngine()
{
    audioProcessor.applyProfileToEngine();
}

void SuperVelocityCurveAudioProcessorEditor::showStatus (const juce::String& message, bool isError)
{
    statusMessage = message;
    statusIsError = isError;
    resized();
    repaint();
}

void SuperVelocityCurveAudioProcessorEditor::updateLiveHits()
{
    svc::HitEvent hit;
    juce::String text;

    int count = 0;
    while (audioProcessor.getEngine().getHitFifo().pop (hit) && count < 8)
    {
        const auto inVel = juce::String (static_cast<int> (std::lround (hit.inputVelocity * 127.0f)));
        const auto outVel = juce::String (static_cast<int> (std::lround (hit.outputVelocity * 127.0f)));
        const auto protocol = hit.isMidi2 ? "M2" : "M1";
        text += "N" + juce::String (hit.note) + " " + protocol + " " + inVel + "→" + outVel + "   ";
        curveEditor.addHitMarker (hit.inputVelocity, hit.outputVelocity);
        padGrid.flashPadHit (hit.note, hit.channel, hit.outputVelocity);
        ++count;
    }

    if (text.isEmpty())
        text = "Play your controller — live input→output velocity appears here";

    liveHitsLabel.setText (text, juce::dontSendNotification);
}

void SuperVelocityCurveAudioProcessorEditor::timerCallback()
{
    updateLiveHits();
    padGrid.decayHitVisuals();
    curveEditor.repaint();
}
