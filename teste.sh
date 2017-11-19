#/bin/bash

touch temp_file

for i in $( ls | grep .jpg  ); do
    echo teste=$i*0.5 > temp_file
    ./main < temp_file
done

rm -f temp_file

rm -rf test_results/

mkdir -p test_results/

mv processos* test_results/
mv thread* test_results/
mv linhas* test_results/
mv colunas* test_results/




