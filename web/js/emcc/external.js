// These are stubs. We are are not using library.js at runtime, so our implementation is not here.
// However, when emcc compiles, it needs these external functions, otherwise it will give warnings. 
mergeInto(LibraryManager.library, { jsDrawRoundedRect: function(){} })
mergeInto(LibraryManager.library, { jsDrawLaneSegment: function(){} })
mergeInto(LibraryManager.library, { jsDrawArcedCorner: function(){} })
mergeInto(LibraryManager.library, { jsDrawRect: function(){} })
mergeInto(LibraryManager.library, { jsDrawLine: function(){} })
mergeInto(LibraryManager.library, { jsDrawEllipse: function(){} })
mergeInto(LibraryManager.library, { jsDrawText: function(){} })
