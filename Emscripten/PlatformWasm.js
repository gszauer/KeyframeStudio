addToLibrary({
    JS_GenGUID: (stringPtr) => {
        // https://stackoverflow.com/questions/105034/how-do-i-create-a-guid-uuid
        let result = "10000000-1000-4000-8000-100000000000".replace(/[018]/g, c =>
            (+c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> +c / 4).toString(16)
        );
        for (let i = 0, size = result.length; i < size; ++i) {
            Module.setValue(stringPtr + i, result.charCodeAt(i), "i8");
        }
    },
    JS_PresentFile: (voidPtr, size) => {
        let memoryArray = new Uint8Array(HEAPU8.buffer, voidPtr, size);// Module.HEAPU8.subarray(voidPtr, size);
        let blobObject = new Blob([memoryArray], {
            type: "application/octet-stream"
        });

        let element = document.createElement('a');
        element.setAttribute('href', window.URL.createObjectURL(blobObject));
        element.setAttribute('download', "KeyFrameStudio");
        element.style.display = 'none';
        document.body.appendChild(element);
        element.click();
        document.body.removeChild(element);
    },
    JS_SelectFile: (callbackPtr) => {
        const FileUploadChangeDetector = () => {
            var fileElement = document.getElementById("fileUploader");
            if (fileElement === null || fileElement === undefined) {
                if (fileElement.requestFileCallback !== null && fileElement.requestFileCallback !== undefined) {
                    console.error("Still waiting for last file");
                    Module._WASM_InvokePlatformSelectCallback(fileElement.requestFileCallback, 0, 0, 0);
                    fileElement.removeEventListener('change', FileUploadChangeDetector);
                    fileElement.value = "";
                    fileElement.requestFileCallback = null;
                    fileElement.addEventListener('change', FileUploadChangeDetector);
                }
                return;
            }

            if (fileElement.files === undefined || fileElement.files.length === 0) {
                console.log("no file");

                fileElement.removeEventListener('change', FileUploadChangeDetector);
                fileElement.value = "";
                fileElement.requestFileCallback = null;
                fileElement.addEventListener('change', FileUploadChangeDetector);
            }
            else {
                let file = fileElement.files[0];
                file.arrayBuffer().then(arrayBuffer=>{
                    let ui8 = new Uint8Array(arrayBuffer);
                    
                    // Marshal file into C++ memory
                    let dst_ptr = Module._malloc(arrayBuffer.byteLength + 1);// memoryAllocator.Realloc(0, arrayBuffer.byteLength + 1);
                    let dst_array = new Uint8Array(HEAPU8.buffer, dst_ptr, arrayBuffer.byteLength);
                    let src_array = new Uint8Array(arrayBuffer);

                    let name_ptr = 0;
                    if (file.name !== undefined && file.name.length > 0) {
                        name_ptr = Module._malloc(file.name.length + 1);// memoryAllocator.Realloc(0, file.name.length + 1);
                        let name_array = new Uint8Array(HEAPU8.buffer, name_ptr, file.name.length + 1);
                        let encoder = new TextEncoder();
                        let name_source = encoder.encode(file.name);
                        for (let i = 0; i < file.name.length; name_array[i] = name_source[i++]);
                        name_array[file.name.length] = 0;
                    }
                    
                    for (let i = 0; i < arrayBuffer.byteLength; dst_array[i] = src_array[i++]);
                    if (fileElement.requestFileCallback !== null && fileElement.requestFileCallback !== undefined) {
                        Module._WASM_InvokePlatformSelectCallback(fileElement.requestFileCallback, name_ptr, dst_ptr, arrayBuffer.byteLength);
                    }
                    if (name_ptr != 0) {
                        //memoryAllocator.Release(name_ptr);
                        Module._free(name_ptr);
                        name_ptr = 0;
                    }
                    if (dst_ptr != 0) {
                        Module._free(dst_ptr);
                        dst_ptr = 0;
                    }

                    fileElement.removeEventListener('change', FileUploadChangeDetector);
                    fileElement.value = "";
                    fileElement.requestFileCallback = null;
                    fileElement.addEventListener('change', FileUploadChangeDetector);
                });
            }
        };

        var fileElement = document.getElementById("fileUploader");
        if (fileElement === null || fileElement === undefined) {
            fileElement = document.createElement('input');
            fileElement.type = "file";
            fileElement.id = "fileUploader";
            fileElement.style.display = 'none';
            document.body.appendChild(fileElement);
            // accept="image/png"
            fileElement.addEventListener('change', FileUploadChangeDetector);
            fileElement.requestFileCallback = null;
        }


        if (fileElement.requestFileCallback != null) {
            console.error("Still waiting for other file");
            
            fileElement.removeEventListener('change', FileUploadChangeDetector);
            fileElement.value = "";
            fileElement.requestFileCallback = null;
            fileElement.addEventListener('change', FileUploadChangeDetector);
        }
        
        fileElement.requestFileCallback = null;
        fileElement.requestFileCallback = callbackPtr;
        fileElement.click();
    },
    JS_OpenURL: (stringPtr) => {
        let stringArray = new Uint8Array(HEAPU8.buffer, stringPtr);
        let stringLength = 0;
        for (let i = 0; i < 256; ++i, ++stringLength) {
            if (stringArray[i] == 0) {
                break;
            }
        }
        stringArray = new Uint8Array(HEAPU8.buffer, stringPtr, stringLength);
        let url = new TextDecoder().decode(stringArray);
        
        window.open(url, "_blank");
    }
  });