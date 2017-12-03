cd result/
for file in ../data/*.rgb
do
    name=${file##*/}

    if [ -d "$name" ]; then
        rm -rf $name
    fi
    mkdir $name
    cd $name

    for i in 1 2 3 4
    do
        mkdir appxl${i}
        cd appxl${i}
        echo "../../../bin/sobel ../../${file} ${i}"
        ../../../bin/sobel ../../${file} ${i}
        cd ../
    done

    cd ../
done
