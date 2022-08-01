#include <infos.h>

int instances = 0;
char FILE_NAME[] = "/usr/exceptions-test";

class A {
public:
    A() {
        instances++;
    }

    //A(A&& a) = delete;
    A(const A &a) {
        instances++;
    }

    A &operator=(const A &a) = delete;

    A &operator=(A &&a) = delete;

    ~A() {
        instances--;
    }

    int val = 9;
};

class Int {
public:
    explicit Int(unsigned int a) {
        n = a;
    }

    unsigned int n;
};

class Char {
public:
    explicit Char(char a) {
        n = a;
    }

    char n;
};

void firstTest() {
    A a;
    try {
        A d;
        throw Int(1);
        A f;
        printf("firstTest after throw executed");
        my_assert(false);
    } catch (Int& a) {
        my_assert(a.n == 1);
        A b;
        printf("firstTest correct catch\n");
    } catch (Char &t) {
        A c;
        printf("firstTest incorrect catch\n");
        my_assert(false);
    }
    my_assert(instances == 1);
}

void secondTest() {
    A a;
    try {
        A b;
        throw Char('a');
    } catch (Int &n) {
        my_assert(false);
    } catch (Char &c) {
        A u;
        my_assert(c.n == 'a');
        printf("secondTest correct catch\n");
    } catch (Char b) {
        my_assert(false);
    }
}

void innerHelper() {
    A a;
    throw A();
}

void middleHelper() {
    try {
        A b;
        innerHelper();
        A m;
    } catch (Int &a) {
        A b;
        my_assert(false);
    }
}

void outerHelper() {
    try {
        A o;
        middleHelper();
    } catch (A& a) {
        printf("thirdTest correct catch\n");
        A y;
    }
}

void thirdTest() {
    outerHelper();
}

void fourthTest() {
    try {
        int f = 0; // does not throw
    } catch (...) {
        printf("fourthTest wrong catch\n");
        my_assert(false);
    }

    try {
        throw A();
    } catch (...) {
        printf("fourthTest correct catch\n");
    }

    try {
        int yr = 0; // does not throw
    } catch (...) {
        printf("fourthTest wrong catch\n");
        my_assert(false);
    }
}

void throw5() {
    throw Int(5);
}

void fifthTest() {
    try {
        innerHelper();
    } catch (A a) {
        A b;
    }

    try {
        throw5();
    } catch (Int& exc) {
        my_assert(exc.n == 5);
        printf("fifthTest correct catch\n");
    }
}

void sixthTest() {
    try {
        try {
            throw A();
        } catch (Int& a) {
            printf("sixthTest wrong catch\n");
            my_assert(false);
        }
    } catch (A& a) {
        printf("sixthTest correct catch\n");
    }
}

void seventhTest() {
    for (int i = 0; i < 100; i++) {
        try {
            throw A();
        } catch (A& a) {

        }
    }
    printf("seventhTest success\n");
}

void test8() {
    bool once = true;
    try {
        try {
            throw A();
            my_assert(false);
        } catch(A &a) {
            my_assert(once);
            once = false;
            throw A();
            my_assert(false);
        } 
    } catch (A& a) {
        printf("test 8 correct catch\n");
    }
}

void test9() {
    int cnt = 2;
    try {
        throw A();
        my_assert(false);
    } catch (A& a) {
        try {
            throw Int(1);
            my_assert(false);
        } catch (Int b) {
            printf("caught int %d\n", b);
            my_assert(b.n == 1);
            cnt--;
        }
        printf("caught A with value %d\n", a.val);
        my_assert(a.val == 9);
        cnt--;
    }
    my_assert(cnt == 0);
    printf("test 9 correct catch\n");
}

int run_tests() {
    firstTest();
    my_assert(instances == 0);
    secondTest();
    my_assert(instances == 0);
    thirdTest();
    my_assert(instances == 0);
    fourthTest();
    my_assert(instances == 0);
    fifthTest();
    my_assert(instances == 0);
    sixthTest();
    my_assert(instances == 0);
    seventhTest();
    my_assert(instances == 0);
    test8();
    my_assert(instances == 0);
    test9();
    my_assert(instances == 0);
	printf("about to throw an Exception that will not be caught\n");
    throw A();
    printf("unreachable\n");
    my_assert(false);
    return 0;
}

int main(const char* cmdline) {
	//prepare_exceptions(FILE_NAME);
	run_tests();
    return 0;
}

