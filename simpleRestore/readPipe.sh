pipe=thePipe
while true
do
    if read line < $pipe; then
        echo "$line"
    fi
done