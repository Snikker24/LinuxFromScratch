for ((i=32;i<255;i++)); do
   printf -v char "\x$(printf '%x' "$i" )"
   as_class=()
   for class in alnum alpha ascii blank cntrl digit graph lower print punct space upper word xdigit; do
     [[ "$char" = @([[:$class:]]) ]] && as_class+=("$class") || as_class+=('')
   done
   printf '%3d | %s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s | %-6s| %-6s | %-6s\n' "$i" "$char" "${as_class[@]}"
done