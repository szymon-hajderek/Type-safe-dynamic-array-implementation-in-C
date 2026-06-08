test: test.c vector.h
	gcc test.c -o test -g3 -std=gnu23 -Wall -Wextra -pedantic -Werror=shadow -Werror=return-type

test-asan: test.c vector.h
	gcc test.c -o test -fsanitize=address,undefined,null -g3 -std=gnu23 -Wall -Wextra -pedantic -Werror=shadow -Werror=return-type