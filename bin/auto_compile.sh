inotifywait -e close_write,moved_to,create -m src |
while read -r directory events filename; do
    # TODO: only react if a cpp or h file changes!
    echo "$filename changed!";
    ./bin/build_js.sh
done
