class AudioDevice {
    constructor(wasmImportObject, allocator, canvasName) {
        if (!wasmImportObject.hasOwnProperty("env")) {
            wasmImportObject.env = {};
        }
        
        this.mem = allocator;
        this.context = new (window.AudioContext || window.webkitAudioContext)();
        this.canvas = document.getElementById(canvasName);
        this.resources = [];
        this.resourceCounter = 1;
        this.u32_max = 4294967295;
        this.listener = this.context.listener;

        let self = this;

        const ResumeAudioContext = function() {
            if (self.context.state === "suspended") {
                self.context.resume();
            }
        }

        this.canvas.addEventListener('touchstart', ResumeAudioContext, true);
        this.canvas.addEventListener('mousedown', ResumeAudioContext, true);

        const GetNextResourceId = function() {
            self.resourceCounter = self.resourceCounter % (self.u32_max - 1) + 1;
            let bufferId = self.resourceCounter;

            let startIndex = bufferId;
            while (self.resources.hasOwnProperty(bufferId)) {
                self.resourceCounter = (self.resourceCounter + 1) % (self.u32_max - 1) + 1;
                bufferId = self.resourceCounter;
                
                if (bufferId == startIndex) {
                    console.error("AudioDevice.GetNextResourceId: ran out of handles");
                    bufferId = 1; // Not 0
                    break;
                }
            }

            self.resources[bufferId] = {
                state: "unset",
                audioBuffer: null, // TODO: rename to just buffer
                source: null, // TODO: rename to just source
                panner2D: null,
                panner3D: null,
                gain: null,
                spatializer: null
            }

            return bufferId;
        }

        const DestroyResource = function(u32_bufferId) {
            if (!self.resources.hasOwnProperty(u32_bufferId)) {
                console.error("AudioDevice.AudioDestroyBuffer: accessing invalid buffer id(" + u32_bufferId + ")");
            }
            let resource = self.resources[u32_bufferId];
            if (resource.state != "ready") {
                console.error("AudioDevice.AudioDestroyBuffer: destroying non-ready buffer id(" + u32_bufferId + ")");
            }

            if (resource.spatializer != null) {
                resource.spatializer.disconnect();
            }
            if (resource.panner2D != null) {
                resource.panner2D.disconnect();
            }
            if (resource.panner3D != null) {
                resource.panner3D.disconnect();
            }
            if (resource.gain != null) {
                resource.gain.disconnect();
            }
            if (resource.source != null) {
                resource.source.stop();
                resource.source.disconnect();
            }

            resource.state = "dead";

            delete self.resources[u32_bufferId];
        };

        wasmImportObject.env["AudioDestroyBuffer"] = DestroyResource;
        wasmImportObject.env["AudioDestroyBus"] = DestroyResource;
        wasmImportObject.env["AudioStop"] = DestroyResource;

        wasmImportObject.env["AudioSetListener"] = function(float_px, float_py, float_pz, float_upx, float_upy, float_upz, float_fwdx, float_fwdy, float_fwdz) {
            if (self.listener.positionX) {
                self.listener.positionX.value = float_px;
                self.listener.positionY.value = float_py;
                self.listener.positionZ.value = float_pz;
            }
            else {
                self.listener.setPosition(float_px, float_py, float_pz);
            }
            if (self.listener.forwardX) {
                self.listener.forwardX.value = float_fwdx;
                self.listener.forwardY.value = float_fwdy;
                self.listener.forwardZ.value = float_fwdz;
                self.listener.upX.value = float_upx;
                self.listener.upY.value = float_upy;
                self.listener.upZ.value = float_upz;
            }
            else {
                self.listener.setOrientation(float_fwdx, float_fwdy, float_fwdz, float_upx, float_upy, float_upz);
            }
        }

        wasmImportObject.env["AudioCreateBuffer"] = function(u32_numChannels, u32_sampleRate, u32_numSamples, ptr_pcmData) {
            let bufferId = GetNextResourceId();
            self.resources[bufferId].state = "loading";

            let pcmData = new Int16Array(self.mem.wasmMemory.buffer, ptr_pcmData, u32_numChannels * u32_numSamples);
            
            const short_range = 32767 + 32768;

            let audioBuffer = self.context.createBuffer(u32_numChannels, u32_numSamples, u32_sampleRate);
            self.resources[bufferId].audioBuffer = audioBuffer;
           
            for (let channel = 0; channel < u32_numChannels; channel++) {
                const channelBuffer = audioBuffer.getChannelData(channel);
                if (u32_numSamples != channelBuffer.length) {
                    console.error("AudioDevice.AudioCreateBuffer buffer has wrong number of channels");
                }

                for (let sample = 0; sample < u32_numSamples; sample++) {
                    const index = sample * u32_numChannels + channel //Math.floor(sample / u32_numChannels) + channel;
                    const shortSample = pcmData[index];
                    let floatSample =  shortSample / short_range;
                    
                    channelBuffer[sample] = floatSample
                }
            }

            self.resources[bufferId].state = "ready";
            
            return bufferId;
        };

        wasmImportObject.env["AudioCreateBus"] = function() {
            let bufferId = GetNextResourceId();

            self.resources[bufferId].state = "ready";
            self.resources[bufferId].panner2D = self.context.createStereoPanner();
            self.resources[bufferId].gain = self.context.createGain();

            self.resources[bufferId].gain.connect(self.resources[bufferId].panner2D);
            self.resources[bufferId].panner2D.connect(self.context.destination);

            return bufferId;
        }

        wasmImportObject.env["AudioSetPan"] = function(u32_soundOrBus, float_pan) {
            if (float_pan < -1.0) {
                float_pan = -1.0;
            }
            if (float_pan > 1.0) {
                float_pan = 1.0;
            }

            if (!self.resources.hasOwnProperty(u32_soundOrBus)) {
                console.error("AudioDevice.AudioSetPan: accessing invalid buffer id(" + u32_soundOrBus + ")");
            }
            if (self.resources[u32_soundOrBus].state != "ready") {
                console.error("AudioDevice.AudioSetPan: accessing non-ready buffer id(" + u32_soundOrBus + ")");
            }

            if (self.resources[u32_soundOrBus].panner2D != null) {
                self.resources[u32_soundOrBus].panner2D.pan.value = float_pan;
            }
        };

        wasmImportObject.env["AudioSetVolume"] = function(u32_soundOrBus, float_volume) {
            if (float_volume < 0.0) {
                float_volume = 0.0;
            }
            if (float_volume > 1.0) {
                float_volume = 1.0;
            }

            if (!self.resources.hasOwnProperty(u32_soundOrBus)) {
                console.error("AudioDevice.AudioSetPan: accessing invalid buffer id(" + u32_soundOrBus + ")");
            }
            if (self.resources[u32_soundOrBus].state != "ready") {
                console.error("AudioDevice.AudioSetPan: accessing non-ready buffer id(" + u32_soundOrBus + ")");
            }

            if (self.resources[u32_soundOrBus].gain != null) {
                self.resources[u32_soundOrBus].gain.gain.value = float_volume;
            }
        }

        wasmImportObject.env["AudioPlay2D"] = function(u32_buffer, u32_bus, bool_looping, float_volume, float_pan) {
            if (float_volume < 0.0) {
                float_volume = 0.0;
            }
            if (float_volume > 1.0) {
                float_volume = 1.0;
            }
            if (float_pan < -1.0) {
                float_pan = -1.0;
            }
            if (float_pan > 1.0) {
                float_pan = 1.0;
            }

            if (!self.resources.hasOwnProperty(u32_bus)) {
                console.error("AudioDevice.AudioPlay2D: accessing invalid buffer id(" + u32_bus + ")");
            }
            let bus = self.resources[u32_bus];
            
            if (!self.resources.hasOwnProperty(u32_buffer)) {
                console.error("AudioDevice.AudioPlay2D: accessing invalid buffer id(" + u32_buffer + ")");
            }
            let buffer = self.resources[u32_buffer];

            if (buffer.state != "ready") {
                console.error("AudioDevice.AudioPlay2D: accessing non-ready buffer id(" + u32_buffer + ")");
            }
            if (buffer.audioBuffer == null) {
                console.error("AudioDevice.AudioPlay2D: null buffer source (" + u32_buffer + ")");
            }
            
            let resourceID = 0;

            if (bool_looping) {
                resourceID = GetNextResourceId();
                let resource = self.resources[resourceID];
                resource.state = "ready";

                resource.panner2D = self.context.createStereoPanner();
                resource.gain = self.context.createGain();

                resource.panner2D.pan.value = float_pan;
                resource.gain.gain.value = float_volume;

                resource.gain.connect(resource.panner2D);
                resource.panner2D.connect(bus.gain);

                resource.source = self.context.createBufferSource();
                resource.source.buffer = buffer.audioBuffer;
                resource.source.connect(resource.gain);
                resource.source.loop = true;
                resource.source.start(0);
            }
            else {
                let oneShot = {
                    source: self.context.createBufferSource(),
                    panner2D: self.context.createStereoPanner(),
                    gain: self.context.createGain()
                };
                oneShot.gain.connect(oneShot.panner2D);
                oneShot.panner2D.connect(bus.gain);
                oneShot.source.connect(oneShot.gain);
                oneShot.gain.gain.value = float_volume;
                oneShot.panner2D.pan.value = float_pan;
                oneShot.source.buffer = buffer.audioBuffer;
                oneShot.source.onended = function() {
                    oneShot.source.disconnect();
                    oneShot.gain.disconnect();
                    oneShot.panner2D.disconnect();
                };
                oneShot.source.start(0);
            }
            
            return resourceID;
        }

        wasmImportObject.env["AudioSetPosition"] = function(u32_sound, float_px, float_py, float_pz) {
            if (!self.resources.hasOwnProperty(u32_sound)) {
                console.error("AudioDevice.AudioSetPosition: accessing invalid buffer id(" + u32_sound + ")");
            }
            if (self.resources[u32_sound].state != "ready") {
                console.error("AudioDevice.AudioSetPosition: accessing non-ready buffer id(" + u32_sound + ")");
            }

            if (self.resources[u32_sound].panner3D != null) {
                self.resources[u32_sound].panner3D.positionX.value = float_px;
                self.resources[u32_sound].panner3D.positionY.value = float_py;
                self.resources[u32_sound].panner3D.positionZ.value = float_pz;
            }
        }

        wasmImportObject.env["AudioSetAttenuation"] = function(u32_sound, float_min, float_max) {
            if (!self.resources.hasOwnProperty(u32_sound)) {
                console.error("AudioDevice.AudioSetAttenuation: accessing invalid buffer id(" + u32_sound + ")");
            }
            if (self.resources[u32_sound].state != "ready") {
                console.error("AudioDevice.AudioSetAttenuation: accessing non-ready buffer id(" + u32_sound + ")");
            }

            if (self.resources[u32_sound].panner3D != null) {
                self.resources[u32_sound].panner3D.refDistance = float_min;
                self.resources[u32_sound].panner3D.maxDistance = float_max;
            }
        }

        wasmImportObject.env["AudioPlay3D"] = function(u32_buffer, u32_bus, bool_looping, float_volume, float_px, float_py, float_pz, float_minattenuation, float_maxattenuation) {
            let resourceID = 0;

            if (float_volume < 0.0) {
                float_volume = 0.0;
            }
            if (float_volume > 1.0) {
                float_volume = 1.0;
            }

            if (!self.resources.hasOwnProperty(u32_bus)) {
                console.error("AudioDevice.AudioPlay2D: accessing invalid buffer id(" + u32_bus + ")");
            }
            let bus = self.resources[u32_bus];
            
            if (!self.resources.hasOwnProperty(u32_buffer)) {
                console.error("AudioDevice.AudioPlay2D: accessing invalid buffer id(" + u32_buffer + ")");
            }
            let buffer = self.resources[u32_buffer];

            if (buffer.state != "ready") {
                console.error("AudioDevice.AudioPlay2D: accessing non-ready buffer id(" + u32_buffer + ")");
            }
            if (buffer.audioBuffer == null) {
                console.error("AudioDevice.AudioPlay2D: null buffer source (" + u32_buffer + ")");
            }

            let pannerNode = new PannerNode(self.context, {
                panningModel: "equalpower",
                distanceModel: "linear",
                positionX: float_px,
                positionY: float_py,
                positionZ: float_pz,
                refDistance: float_minattenuation,
                maxDistance: float_maxattenuation,
                rolloffFactor: 1.0,
            });

            if (bool_looping) {
                resourceID = GetNextResourceId();
                let resource = self.resources[resourceID];
                resource.state = "ready";

                resource.panner3D = pannerNode;
                resource.gain = self.context.createGain();

                resource.gain.gain.value = float_volume;

                resource.gain.connect(resource.panner3D);
                resource.panner3D.connect(bus.gain);

                resource.source = self.context.createBufferSource();
                resource.source.buffer = buffer.audioBuffer;
                resource.source.connect(resource.gain);
                resource.source.loop = true;
                resource.source.start(0);
            }
            else {
                let oneShot = {
                    source: self.context.createBufferSource(),
                    panner3D: pannerNode,
                    gain: self.context.createGain()
                };
                oneShot.gain.connect(oneShot.panner3D);
                oneShot.panner3D.connect(bus.gain);
                oneShot.source.connect(oneShot.gain);
                oneShot.gain.gain.value = float_volume;
                oneShot.source.buffer = buffer.audioBuffer;
                oneShot.source.onended = function() {
                    oneShot.source.disconnect();
                    oneShot.gain.disconnect();
                    oneShot.panner3D.disconnect();
                };
                oneShot.source.start(0);
            }
            
            return resourceID;
        }
    }
}