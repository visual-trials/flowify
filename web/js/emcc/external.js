// These are stubs. We are are not using library.js at runtime, so our implementation is not here.
// However, when emcc compiles, it needs these external functions, otherwise it will give warnings. 
mergeInto(LibraryManager.library, { jsDrawRoundedRect: function(){} })
mergeInto(LibraryManager.library, { jsDrawRect: function(){} })
mergeInto(LibraryManager.library, { jsDrawText: function(){} })
