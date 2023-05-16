class FileLoader {
    constructor(packageName, wasmImportObject, memoryAllocator) {
        if (!wasmImportObject.hasOwnProperty("env")) {
            wasmImportObject.env = {};
        }
        let self = this;

        //<input  type="file" id="fileElem" accept="image/png" style="display:none" />
        this.fileElement = document.getElementById("fileUploader"); // This should just exist

        this.callbacksEnabled = false;
        this.queuedCallbacks = null;
        this.wasmInstance = null;
        this.mem_u8 = memoryAllocator.mem_u8;
        this.mem_buffer = memoryAllocator.wasmMemory.buffer;
        this.decoder = new TextDecoder();
        this.encoder = new TextEncoder();

        this.requestFileCallback = null;
        this.requestFileUserData = null;

        const FileUploadChangeDetector = () => {
            if (self.fileElement.files === undefined || self.fileElement.files.length === 0) {
                console.log("no file");

                self.fileElement.removeEventListener('change', FileUploadChangeDetector);
                self.fileElement.value = "";
                self.fileElement.addEventListener('change', FileUploadChangeDetector);
            }
            else {
                self.requestFileCallback
                self.requestFileUserData

                let file = self.fileElement.files[0];
                file.arrayBuffer().then(arrayBuffer=>{
                    let ui8 = new Uint8Array(arrayBuffer);
                    
                    // Marshal file into C++ memory
                    let dst_ptr = memoryAllocator.Realloc(0, arrayBuffer.byteLength + 1);
                    let dst_array = new Uint8Array(self.mem_buffer, dst_ptr, arrayBuffer.byteLength);
                    let src_array = new Uint8Array(arrayBuffer);

                    let name_ptr = 0;
                    if (file.name !== undefined && file.name.length > 0) {
                        name_ptr = memoryAllocator.Realloc(0, file.name.length + 1);
                        let name_array = new Uint8Array(self.mem_buffer, name_ptr, file.name.length + 1);
                        let name_source = self.encoder.encode(file.name);
                        for (let i = 0; i < file.name.length; name_array[i] = name_source[i++]);
                        name_array[file.name.length] = 0;
                    }
                    
                    for (let i = 0; i < arrayBuffer.byteLength; dst_array[i] = src_array[i++]);
                    self.wasmInstance.exports.TriggerFileLoaderCallback(self.requestFileCallback, name_ptr, dst_ptr, arrayBuffer.byteLength, self.requestFileUserData);

                    if (name_ptr != 0) {
                        memoryAllocator.Release(name_ptr);
                    }

                    self.requestFileCallback = null;
                    self.requestFileUserData = null;

                    self.fileElement.removeEventListener('change', FileUploadChangeDetector);
                    self.fileElement.value = "";
                    self.fileElement.addEventListener('change', FileUploadChangeDetector);
                });
            }
        }
        
        this.fileElement.addEventListener('change', FileUploadChangeDetector);

        wasmImportObject.env["RequestFileAsynch"] = function(ptr_callback, ptr_userData) {
            if (self.requestFileCallback != null ||  self.requestFileUserData != null) {
                console.error("Still waiting for other file");
                
                self.fileElement.removeEventListener('change', FileUploadChangeDetector);
                self.fileElement.value = "";
                self.fileElement.addEventListener('change', FileUploadChangeDetector);
            }
            
            if (self.fileElement) {
                self.requestFileCallback = ptr_callback;
                self.requestFileUserData = ptr_userData;
                self.fileElement.click();
            }
        }

        wasmImportObject.env["LoadFileAsynch"] = function(_path, target, bytes, callback, userData) {
            let iter = _path;
            while(self.mem_u8[iter] != 0) {
                iter += 1;
                if (iter - _path > 5000) {
                    console.error("FileLoader.FileLoad string decode loop took too long");
                    break;
                }
            }
            let stringPath = self.decoder.decode(new Uint8Array(self.mem_buffer, _path, iter - _path));
            if (stringPath == null || stringPath.length == 0) {
                console.error("FileLoader.FileLoad file path was empty, pointer:" + _path);
            }

            self.LoadFile(_path, stringPath, target, bytes, callback, userData);
        };

        const SaveFileByName = function(stringPath, ptr_data, u32_bytes) {
            let memoryArray = new Uint8Array(self.mem_buffer, ptr_data, u32_bytes);
            var blobObject = new Blob([memoryArray], {type: "application/octet-stream"});

            let element = document.createElement('a');
            element.setAttribute('href', window.URL.createObjectURL(blobObject));
            element.setAttribute('download', stringPath);
            element.style.display = 'none';
            document.body.appendChild(element);
            element.click();
            document.body.removeChild(element);
        }

        const RequestFileAsynch = function(ptr_callback, ptr_userdata) {

        }

        wasmImportObject.env["SaveFile"] = function(ptr_path, ptr_data, u32_bytes) {
            let iter = ptr_path;
            while(self.mem_u8[iter] != 0) {
                iter += 1;
                if (iter - _path > 5000) {
                    console.error("FileLoader.SaveFile string decode loop took too long");
                    break;
                }
            }
            let stringPath = self.decoder.decode(new Uint8Array(self.mem_buffer, ptr_path, iter - _path));
            if (stringPath == null || stringPath.length == 0) {
                console.error("FileLoader.SaveFile file path was empty, pointer:" + _path);
            }

            SaveFileByName(stringPath, ptr_data, u32_bytes);
        };

        wasmImportObject.env["PresentFile"] = function(ptr_data, u32_bytes) {
            SaveFileByName("file.dat", ptr_data, u32_bytes);
            return true;
        };
    }

    AttachToWasmInstance(wasmInstance) {
        this.wasmInstance = wasmInstance;

        this.callbacksEnabled = true;
        if (this.queuedCallbacks != null) {
            while (this.queuedCallbacks.length != 0) {
                let callback = this.queuedCallbacks.pop();
                this.LoadFile(callback.path, callback.stringPath, callback.target, callback.bytes, callback.callback, callback.userData);
            }
            this.queuedCallbacks = null;
        }
    }

    LoadFile(_path, _stringPath, _target, _bytes, _callback, _userData) {
        //console.log("Loading: " + _stringPath);

        let self = this;
        if (!this.callbacksEnabled) {
            if (this.queuedCallbacks == null) {
                this.queuedCallbacks = [];
            }
            let callback = {
                path: _path,
                stringPath: _stringPath,
                target: _target,
                bytes: _bytes,
                callback: _callback,
                userData: _userData
            };
            this.queuedCallbacks.push(callback);
            console.error("file queued");
        }
        else {
            this.LoadFileAsArrayBuffer(_stringPath, function(path, arrayBuffer) {
                let writtenBytes = 0;
                if (arrayBuffer == null) {
                    _target = 0; // Set data to null
                }
                else {
                    let dst_array = new Uint8Array(self.mem_buffer, _target, _bytes);
                    let src_array = new Uint8Array(arrayBuffer);
                    
                    writtenBytes = _bytes < arrayBuffer.byteLength? _bytes : arrayBuffer.byteLength;
                    for (let i = 0; i < writtenBytes; dst_array[i] = src_array[i++]);
                }
                self.wasmInstance.exports.TriggerFileLoaderCallback(_callback, _path, _target, writtenBytes, _userData);
            });
        }
    }

    LoadFileAsArrayBuffer(path, onFileLoaded) { 
        const req = new XMLHttpRequest();
        req.customCallbackTriggered = false;
        req.open('GET', path, true);
        req.responseType = "arraybuffer";
        let self = this;

        req.onload = (event) => {
            const arrayBuffer = req.response; // Note: not req.responseText
            if (arrayBuffer) {
                if (!req.customCallbackTriggered ) {
                    req.customCallbackTriggered  = true;
                    onFileLoaded(path, arrayBuffer);
                }
            }
            else {
                if (!req.customCallbackTriggered ) {
                    console.error("FileLoader.LoadFileAsArrayBuffer Could not load: " + path);
                    req.customCallbackTriggered  = true;
                    onFileLoaded(path, null);
                }
            }
        };

        req.send(null);
    }
}