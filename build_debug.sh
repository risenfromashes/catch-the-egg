# arg1 workspaceFolder
# arg2 relative dirname
# arg3 relative file Path
# arg4 file Base Name
# arg5 run?
cd $1

if [[ "$OSTYPE" == "linux-gnu"* ]]
then 
    g++ -g -IOpenGL/include -LOpenGL/dll/x64 $3  -o ./bin/$4 -lGL -lglut
    ./bin/$4
else 
    export PATH=/c/msys64/mingw64/bin:$PATH
    g++ -g -I./include -L./dll/x64 $3 -o ./bin/$4 -lmingw32 -lfreeglut -lOPENGL32 -lSDL2main -lSDL2 -lSDL2_mixer -O2 # -mwindows
    for file in dll/x64/*.dll
    do
        cp $file ./bin/
    done

    if [[ "$5" == "YES" ]]
    then
        ./bin/$4.exe
    fi
fi