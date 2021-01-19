setsid ./tasksWithPipe.out < /dev/null &
pid=$!
echo "PID: $pid"

pipe=thePipe
while true
do
    if read line < $pipe; then
        cmd=$(echo "$line" | cut -c 1-4)
        arg=$(echo "$line" | cut -c 6-)
        if [[ "$cmd" == 'exit' ]]; then
            echo "Exit" && exit
        elif [[ "$cmd" == 'save' ]]; then
            echo "Save $arg"
            criu dump -v2 -D "$arg" -R -j -t "$pid" -o d.log
        elif [[ "$cmd" == 'rest' ]]; then
            echo "Restore $arg"
            kill -9 "$pid"
            criu restore -v2 -D "$arg" -j
        fi
    fi
done
