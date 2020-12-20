# arg1 workspaceFolder
# arg2 relative dirname
# arg3 relative file Path
# arg4 file Base Name
# arg5 build target x86/x64
cd "$1"
if [[ "$5" == "x64" ]]
then 
    export PATH=/c/msys64/mingw64/bin:$PATH
else
    export PATH=/c/msys64/mingw32/bin:$PATH
fi
g++ -Iinclude -Ldll/$5 $3  -o ./bin/$5/$4.exe -mwindows -O3 -static-libgcc -static-libstdc++ -lfreeglut -lOPENGL32 -lgdi32 -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer

cd $2
7z a -tzip $4_src.zip $4.cpp #add other source files here
cd "$1"
mv $2/$4_src.zip ./release
cd "$1"
for file in ./dll/$5/*.dll
do
    cp $file ./bin/$5
done
cd bin/$5
7z a -tzip $4_win_$5.zip $4.exe ./*.dll ../../assets
mv $4_win_$5.zip ../../release
