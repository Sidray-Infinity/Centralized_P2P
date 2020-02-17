Splitting the file:
split <filename> $filename -b <blocksize> -d --verbose

Merging blocks:
cat file1 file2 ... > target_file
chmod 777 target_file