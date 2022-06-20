mkdir libs

while read file; do
    cp -v $file libs/`basename $file`
done < ../appveyor_files/mac_cli_dylibs.txt

while read file; do
    ls libs | xargs -I {} install_name_tool -change $file @executable_path/libs/`basename $file` libs/{}
    install_name_tool -change $file @executable_path/libs/`basename $file` CHOpt
done < ../appveyor_files/mac_cli_dylibs.txt