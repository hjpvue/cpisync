Notes on wrapping for JS

First step might be to build from under Node and see if the build process itself works that way.
And then - whether we can run *some* C code from the resulting library.

Next we plan to play with wrapping a useful subset of the CuckooSync API.

I wonder how to deal with NTL, cppunit and GMP dependencies under Node?
Should probably download and build it automatically from the Node build script?
We can run cmake first and see if it flies, maybe it'll find the system libntl-dev and libcppunit-dev and libgmp-dev or other such versions.
If the preliminary build fails, we can fall back to downloading the dependencies and pointing cmake at them.

we are porting to dll and then calling node native