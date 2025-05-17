# Realtime OBS translation + streaming plugin

## Compiled Latest Version of the Solution

### 1. Overall Plugin Concept

- **Plugin Type:**  
  An OBS Source plugin named “API Multi-Language Voice & Text Source” responsible for:
  - Receiving voice data and API responses.
  - Generating a multi-channel audio stream.
  - Displaying time-synced text (captions) within OBS.

- **Key Consideration:**  
  The plugin does not need to implement NDI output directly. Instead, it produces correctly formatted multi-channel audio and text output that can be picked up by a separate, fully featured OBS NDI plugin (e.g., “obs-ndi”).

---

### 2. Modules and Components

#### A. Audio Capture Module
- Captures input audio (from the speaker/admin client).
- Prepares audio data for API requests and subsequent processing.

#### B. Network Client Module
- Handles all API communications with the central server.
- RSS/HTTP interactions include:
  - **Transcription:** Upload captured audio for language-specific speech-to-text processing.
  - **Translation:** Retrieve translated captions in target languages.
  - **Text-to-Speech (TTS):** Optionally retrieve TTS audio.
- Parses multi-language responses and tags audio streams with their corresponding language IDs.

#### C. Audio Playback Module
- **Channel Mapping:**  
  Receives tagged audio from the network client and distributes it to pre-designated output channels based on a user-defined mapping.
- **Multi-channel Output:**  
  Presents interleaved multi-channel audio through the OBS audio_render callback.
- **NDI Compatibility:**  
  Produces standard multi-channel audio acceptable to the `obs-ndi` plugin’s “Source Output” procedure.

#### D. Text Output Module
- **Primary Function:**  
  Extracts and passes through the filtered text (captions) for the primary language.
- **Output Options:**  
  - Writes to a file (for external viewing or network shares).
  - Updates a specified OBS Text Source dynamically within the plugin’s scene.
- **Note on NDI:**  
  The text is not embedded within the NDI stream. Any text display isn’t handed over to external NDI viewers like VLC automatically.

#### E. Configuration UI (Properties Panel)
- **Inputs & Endpoints:**  
  - Audio source settings.
  - API endpoint URL and authentication.
- **Channel Mapping:**  
  Allows mapping which language (or voice channel) goes to which output channel.
- **Text Options:**  
  - Enable/Disable text output.
  - Select primary text language.
  - Choose method: writing to file or updating an OBS text source.
- **Status & Enable/Disable Toggle:**  
  For monitoring and quick adjustments.

#### F. State Management Module
- Maintains the operational state and synchronizes audio/text across changes in network and user configuration.

---

### 3. Integration with OBS NDI Workflow

#### A. User Setup Workflow:
1. **Install Prerequisites:**
   - Install our API Multi-Language Voice & Text Source plugin.
   - Install the OBS NDI plugin (e.g., “obs-ndi”).

2. **Configure Our Plugin in OBS:**
   - Add the source to the desired scene.
   - Set up API details, language mapping, and text output paths or targets.

3. **Configure OBS NDI Output:**
   - Navigate to OBS → Tools → NDI Output Settings.
   - Enable a “Source Output” and select the instance of our plugin as the source.
   - Assign a stream name (e.g., “OBS-API-Audio”).

4. **Client (VLC) Setup:**
   - In VLC’s “Open Network Stream” section, let it discover and display available NDI sources.
   - Select the correct named NDI stream; note that VLC will pick up the multi-channel audio (subject to VLC’s handling of multi-channel streams).
   - For text, view the separately provided text file or have OBS display the text within a scene.

#### B. Handling Output Differences:
- **Audio:**  
  Multi-channel audio is forwarded through the standard audio render callback and is completely compliant with what the OBS NDI plugin expects.
- **Text:**  
  Since NDI outputs don’t handle arbitrary text streams well:
  - The text transcription is provided separately (via file or OBS text source update).
  - Users need to access caption text by opening the generated file or via the OBS scene itself.

---

### 4. Development & Deployment Considerations

- **Separation of Concerns:**  
  Keep core functionality (network processing, audio capture, multi-channel output) within our custom source plugin. Leave the transport (NDI streaming) to the robust, dedicated OBS `obs-ndi` plugin.
  
- **Testing:**
  - Verify that the multi-channel audio from our plugin is correctly recognized and transmitted by the OBS NDI output.
  - Test using VLC or NDI Studio Monitor to ensure proper channel allocation.
  - Validate text output methods separately to guarantee synchronization with audio despite the two being delivered via different systems.

- **Packaging & Documentation:**
  - Provide clear installation instructions.
  - Document the necessity for the OBS NDI plugin.
  - Step-by-step guides for configuring both our plugin and the NDI output.
  - Explain that text output is not part of the NDI stream and must be accessed via provided alternatives.
