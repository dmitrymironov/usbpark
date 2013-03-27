#valgrind -v -v -v --tool=memcheck --log-file=valgr --leak-check=yes --num-callers=10 --suppressions=parkerD.sup --gen-suppressions=yes $INSTALL_BIN/parkerD --enable-stdin
valgrind -v -v -v --tool=memcheck --db-attach=yes --db-command='gdb --readnow -nw %f %p' --leak-check=yes --num-callers=10 --suppressions=parkerD.sup $INSTALL_BIN/parkerD
