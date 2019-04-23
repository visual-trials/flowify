// These are stubs. We are are not using library.js at runtime, so our implementation is not here.
// However, when emcc compiles, it needs these external functions, otherwise it will give warnings. 
mergeInto(LibraryManager.library, { jsClipRect: function(){} })
mergeInto(LibraryManager.library, { jsUnClipRect: function(){} })
mergeInto(LibraryManager.library, { jsDrawRoundedRect: function(){} })
mergeInto(LibraryManager.library, { jsDrawLane: function(){} })
mergeInto(LibraryManager.library, { jsDrawRect: function(){} })
mergeInto(LibraryManager.library, { jsDrawLine: function(){} })
mergeInto(LibraryManager.library, { jsDrawEllipse: function(){} })
mergeInto(LibraryManager.library, { jsDrawText: function(){} })
mergeInto(LibraryManager.library, { jsGetTextWidth: function(){} })
mergeInto(LibraryManager.library, { jsLog: function(){} })
mergeInto(LibraryManager.library, { jsAbort: function(){} })
mergeInto(LibraryManager.library, { jsSetUsingPhysicalPixels: function(){} })

