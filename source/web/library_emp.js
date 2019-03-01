//  This file is part of Empirical, https://github.com/devosoft/Empirical
//  Copyright (C) Michigan State University, 2015-2018.
//  Released under the MIT Software license; see doc/LICENSE

mergeInto(LibraryManager.library, {
    // Data accessible to library users.
    $emp: {
        Callback: function() {
            // Copy over the additional arguments
            emp_i.cb_args = [];
            emp_i.cb_return = 0;
            for (var i = 1; i < arguments.length; i++) {
                emp_i.cb_args[i-1] = arguments[i];
            }

            // Callback to the original function.
            empCppCallback(arguments[0]);

            return emp_i.cb_return;
        },

        InspectObj: function(o,i) {
            // From: http://stackoverflow.com/questions/5357442/how-to-inspect-javascript-objects
            if (typeof i == 'undefined') i='';
            if (i.length > 50) return '[MAX ITERATIONS]';
            var r = [];
            for(var p in o){
                var t = typeof o[p];
                r.push(i + '"' + p + '" (' + t + ') => '
                       + (t == 'object' ? 'object:' + InspectObj(o[p], i+'  ') : o[p] + ''));
            }
            return r.join(i+'\n');
        },

        LoadFileEvent: function(files, callback_id) {
            var reader = new FileReader();            // Reader object
            reader.onload = function(e) {             // Fun to run when file loaded
                emp.Callback(callback_id, e.target.result + '\n');   // Do callback!
            };
            reader.readAsText(files[0]);   // Load file!
        },

        // The saveAs() Function is from FileSaver.js 1.3.2 by Eli Grey, http://eligrey.com
        saveAs: function(view) {
          "use strict";
          // IE <10 is explicitly unsupported
          if (typeof view === "undefined" || typeof navigator !== "undefined" && /MSIE [1-9]\./.test(navigator.userAgent)) {
            return;
	        }
          var
          doc = view.document
          // only get URL when necessary in case Blob.js hasn't overridden it yet
          , get_URL = function() { return view.URL || view.webkitURL || view; }
          , save_link = doc.createElementNS("http://www.w3.org/1999/xhtml", "a")
          , can_use_save_link = "download" in save_link
          , click = function(node) {
            var event = new MouseEvent("click");
			      node.dispatchEvent(event);
          }
          , is_safari = /constructor/i.test(view.HTMLElement) || view.safari
          , is_chrome_ios =/CriOS\/[\d]+/.test(navigator.userAgent)
          , throw_outside = function(ex) {
			      (view.setImmediate || view.setTimeout)(function() {
				      throw ex;
			      }, 0);
          }
		      , force_saveable_type = "application/octet-stream"
		      // the Blob API is fundamentally broken as there is no "downloadfinished" event to subscribe to
	      	, arbitrary_revoke_timeout = 1000 * 40 // in ms
	      	, revoke = function(file) {
	      		var revoker = function() {
	      			if (typeof file === "string") { // file is an object URL
	      				get_URL().revokeObjectURL(file);
	      			} else { // file is a File
	      				file.remove();
	      			}
	      		};
	      		setTimeout(revoker, arbitrary_revoke_timeout);
	      	}
	      	, dispatch = function(filesaver, event_types, event) {
	      		event_types = [].concat(event_types);
	      		var i = event_types.length;
	      		while (i--) {
	      			var listener = filesaver["on" + event_types[i]];
	      			if (typeof listener === "function") {
	      				try {
	      					listener.call(filesaver, event || filesaver);
	      				} catch (ex) {
	      					throw_outside(ex);
	      				}
	      			}
	      		}
	      	}
	      	, auto_bom = function(blob) {
	      		// prepend BOM for UTF-8 XML and text/* types (including HTML)
	      		// note: your browser will automatically convert UTF-16 U+FEFF to EF BB BF
	      		if (/^\s*(?:text\/\S*|application\/xml|\S*\/\S*\+xml)\s*;.*charset\s*=\s*utf-8/i.test(blob.type)) {
	      			return new Blob([String.fromCharCode(0xFEFF), blob], {type: blob.type});
	      		}
	      		return blob;
	      	}
	      	, FileSaver = function(blob, name, no_auto_bom) {
	      		if (!no_auto_bom) {
	      			blob = auto_bom(blob);
	      		}
	      		// First try a.download, then web filesystem, then object URLs
	      		var
	      			  filesaver = this
	      			, type = blob.type
	      			, force = type === force_saveable_type
	      			, object_url
	      			, dispatch_all = function() {
	      				dispatch(filesaver, "writestart progress write writeend".split(" "));
	      			}
	      			// on any filesys errors revert to saving with object URLs
	      			, fs_error = function() {
	      				if ((is_chrome_ios || (force && is_safari)) && view.FileReader) {
	      					// Safari doesn't allow downloading of blob urls
				      		var reader = new FileReader();
				      		reader.onloadend = function() {
				      			var url = is_chrome_ios ? reader.result : reader.result.replace(/^data:[^;]*;/, 'data:attachment/file;');
				      			var popup = view.open(url, '_blank');
				      			if(!popup) view.location.href = url;
				      			url=undefined; // release reference before dispatching
				      			filesaver.readyState = filesaver.DONE;
				      			dispatch_all();
				      		};
				      		reader.readAsDataURL(blob);
				      		filesaver.readyState = filesaver.INIT;
				      		return;
				      	}
				      	// don't create more object URLs than needed
				      	if (!object_url) {
				      		object_url = get_URL().createObjectURL(blob);
				      	}
				      	if (force) {
				      		view.location.href = object_url;
				      	} else {
				      		var opened = view.open(object_url, "_blank");
				      		if (!opened) {
				      			// Apple does not allow window.open, see https://developer.apple.com/library/safari/documentation/Tools/Conceptual/SafariExtensionGuide/WorkingwithWindowsandTabs/WorkingwithWindowsandTabs.html
				      			view.location.href = object_url;
				      		}
					      }
		      			filesaver.readyState = filesaver.DONE;
		      			dispatch_all();
		      			revoke(object_url);
		      		}
		      	;
		      	filesaver.readyState = filesaver.INIT;

			      if (can_use_save_link) {
			      	object_url = get_URL().createObjectURL(blob);
			      	setTimeout(function() {
			      		save_link.href = object_url;
			      		save_link.download = name;
			      		click(save_link);
			      		dispatch_all();
			      		revoke(object_url);
			      		filesaver.readyState = filesaver.DONE;
			      	});
			      	return;
			      }

			      fs_error();
		      }
      		, FS_proto = FileSaver.prototype
      		, saveAs = function(blob, name, no_auto_bom) {
      			return new FileSaver(blob, name || blob.name || "download", no_auto_bom);
      		}
	      ;
	      // IE 10+ (native saveAs)
	      if (typeof navigator !== "undefined" && navigator.msSaveOrOpenBlob) {
	      	return function(blob, name, no_auto_bom) {
	      		name = name || blob.name || "download";

	      		if (!no_auto_bom) {
	      			blob = auto_bom(blob);
	      		}
	      		return navigator.msSaveOrOpenBlob(blob, name);
	      	};
	      }

      	FS_proto.abort = function(){};
      	FS_proto.readyState = FS_proto.INIT = 0;
      	FS_proto.WRITING = 1;
      	FS_proto.DONE = 2;

      	FS_proto.error =
      	FS_proto.onwritestart =
      	FS_proto.onprogress =
      	FS_proto.onwrite =
      	FS_proto.onabort =
      	FS_proto.onerror =
      	FS_proto.onwriteend =
		      null;

      	return saveAs;
      }(
      	   typeof self !== "undefined" && self
      	|| typeof window !== "undefined" && window
      	|| this.content
      ),

      //download.js v4.21, by dandavis; 2008-2018. [MIT] see http://danml.com/download.html for tests/usage
      // v1 landed a FF+Chrome compatible way of downloading strings to local un-named files, upgraded to use a hidden frame and optional mime
      // v2 added named files via a[download], msSaveBlob, IE (10+) support, and window.URL support for larger+faster saves than dataURLs
      // v3 added dataURL and Blob Input, bind-toggle arity, and legacy dataURL fallback was improved with force-download mime and base64 support. 3.1 improved safari handling.
      // v4 adds AMD/UMD, commonJS, and plain browser support
      // v4.1 adds url download capability via solo URL argument (same domain/CORS only)
      // v4.2 adds semantic variable names, long (over 2MB) dataURL support, and hidden by default temp anchors
      // https://github.com/rndme/download
      download: function(data, strFileName, strMimeType) {

      	var self = window, // this script is only for browsers anyway...
      		defaultMime = "application/octet-stream", // this default mime also triggers iframe downloads
      		mimeType = strMimeType || defaultMime,
      		payload = data,
      		url = !strFileName && !strMimeType && payload,
      		anchor = document.createElement("a"),
      		toString = function(a){return String(a);},
      		myBlob = (self.Blob || self.MozBlob || self.WebKitBlob || toString),
      		fileName = strFileName || "download",
      		blob,
      		reader;
      		myBlob= myBlob.call ? myBlob.bind(self) : Blob ;

      	if(String(this)==="true"){ //reverse arguments, allowing download.bind(true, "text/xml", "export.xml") to act as a callback
      		payload=[payload, mimeType];
      		mimeType=payload[0];
      		payload=payload[1];
      	}


      	if(url && url.length< 2048){ // if no filename and no mime, assume a url was passed as the only argument
      		fileName = url.split("/").pop().split("?")[0];
      		anchor.href = url; // assign href prop to temp anchor
      	  	if(anchor.href.indexOf(url) !== -1){ // if the browser determines that it's a potentially valid url path:
            		var ajax=new XMLHttpRequest();
            		ajax.open( "GET", url, true);
            		ajax.responseType = 'blob';
            		ajax.onload= function(e){
      			  download(e.target.response, fileName, defaultMime);
      			};
            		setTimeout(function(){ ajax.send();}, 0); // allows setting custom ajax headers using the return:
      		    return ajax;
      		} // end if valid url?
      	} // end if url?


      	//go ahead and download dataURLs right away
      	if(/^data:([\w+-]+\/[\w+.-]+)?[,;]/.test(payload)){

      		if(payload.length > (1024*1024*1.999) && myBlob !== toString ){
      			payload=dataUrlToBlob(payload);
      			mimeType=payload.type || defaultMime;
      		}else{
      			return navigator.msSaveBlob ?  // IE10 can't do a[download], only Blobs:
      				navigator.msSaveBlob(dataUrlToBlob(payload), fileName) :
      				saver(payload) ; // everyone else can save dataURLs un-processed
      		}

      	}else{//not data url, is it a string with special needs?
      		if(/([\x80-\xff])/.test(payload)){
      			var i=0, tempUiArr= new Uint8Array(payload.length), mx=tempUiArr.length;
      			for(i;i<mx;++i) tempUiArr[i]= payload.charCodeAt(i);
      		 	payload=new myBlob([tempUiArr], {type: mimeType});
      		}
      	}
      	blob = payload instanceof myBlob ?
      		payload :
      		new myBlob([payload], {type: mimeType}) ;


      	function dataUrlToBlob(strUrl) {
      		var parts= strUrl.split(/[:;,]/),
      		type= parts[1],
      		indexDecoder = strUrl.indexOf("charset")>0 ? 3: 2,
      		decoder= parts[indexDecoder] == "base64" ? atob : decodeURIComponent,
      		binData= decoder( parts.pop() ),
      		mx= binData.length,
      		i= 0,
      		uiArr= new Uint8Array(mx);

      		for(i;i<mx;++i) uiArr[i]= binData.charCodeAt(i);

      		return new myBlob([uiArr], {type: type});
      	 }

      	function saver(url, winMode){

      		if ('download' in anchor) { //html5 A[download]
      			anchor.href = url;
      			anchor.setAttribute("download", fileName);
      			anchor.className = "download-js-link";
      			anchor.innerHTML = "downloading...";
      			anchor.style.display = "none";
      				anchor.addEventListener('click', function(e) {
      					e.stopPropagation();
      					this.removeEventListener('click', arguments.callee);
      				});
      			document.body.appendChild(anchor);
      			setTimeout(function() {
      				anchor.click();
      				document.body.removeChild(anchor);
      				if(winMode===true){setTimeout(function(){ self.URL.revokeObjectURL(anchor.href);}, 250 );}
      			}, 66);
      			return true;
      		}

      		// handle non-a[download] safari as best we can:
      		if(/(Version)\/(\d+)\.(\d+)(?:\.(\d+))?.*Safari\//.test(navigator.userAgent)) {
      			if(/^data:/.test(url))	url="data:"+url.replace(/^data:([\w\/\-\+]+)/, defaultMime);
      			if(!window.open(url)){ // popup blocked, offer direct download:
      				if(confirm("Displaying New Document\n\nUse Save As... to download, then click back to return to this page.")){ location.href=url; }
      			}
      			return true;
      		}

      		//do iframe dataURL download (old ch+FF):
      		var f = document.createElement("iframe");
      		document.body.appendChild(f);

      		if(!winMode && /^data:/.test(url)){ // force a mime that will download:
      			url="data:"+url.replace(/^data:([\w\/\-\+]+)/, defaultMime);
      		}
      		f.src=url;
      		setTimeout(function(){ document.body.removeChild(f); }, 333);

      	}//end saver




      	if (navigator.msSaveBlob) { // IE10+ : (has Blob, but not a[download] or URL)
      		return navigator.msSaveBlob(blob, fileName);
      	}

      	if(self.URL){ // simple fast and modern way using Blob and URL:
      		saver(self.URL.createObjectURL(blob), true);
      	}else{
      		// handle non-Blob()+non-URL browsers:
      		if(typeof blob === "string" || blob.constructor===toString ){
      			try{
      				return saver( "data:" +  mimeType   + ";base64,"  +  self.btoa(blob)  );
      			}catch(y){
      				return saver( "data:" +  mimeType   + "," + encodeURIComponent(blob)  );
      			}
      		}

      		// Blob but not URL support:
      		reader=new FileReader();
      		reader.onload=function(e){
      			saver(this.result);
      		};
      		reader.readAsDataURL(blob);
      	}
      	return true;
      } /* end download() */

    },

    // Data internal to EMP
    $emp_i: { cb_args:[], cb_return:0, images:[]
            },

    EMP_Initialize__deps: ['$emp', '$emp_i'],
    EMP_Initialize: function () {
        empCppCallback = Module.cwrap('empCppCallback', null, ['number']);
    },

    EMP_GetCBArgCount__deps: ['$emp', '$emp_i'],
    EMP_GetCBArgCount: function() { return emp_i.cb_args.length; },
});
