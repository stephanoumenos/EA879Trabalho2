#/bin/bash

rm -f temp_file

touch temp_file

for i in $( ls test_images/ ); do
    echo -n test_images/ >> temp_file
    echo i >> temp_file
done

# Arquivo com nome das fotos pronto

./main < temp_file # Usa como entrada para o programa


