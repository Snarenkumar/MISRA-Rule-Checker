// Sample file demonstrating all 15 MISRA/AUTOSAR rule violations

// 1. NO_USING_NAMESPACE
using namespace std;

// 2. NO_RECURSION
void recursive_function() {
    recursive_function();
}

// 3. MAX_FUNC_LENGTH (> 50 lines)
void overly_long_function() {
    int v1 = 1;
    int v2 = 2;
    int v3 = 3;
    int v4 = 4;
    int v5 = 5;
    int v6 = 6;
    int v7 = 7;
    int v8 = 8;
    int v9 = 9;
    int v10 = 10;
    int v11 = 11;
    int v12 = 12;
    int v13 = 13;
    int v14 = 14;
    int v15 = 15;
    int v16 = 16;
    int v17 = 17;
    int v18 = 18;
    int v19 = 19;
    int v20 = 20;
    int v21 = 21;
    int v22 = 22;
    int v23 = 23;
    int v24 = 24;
    int v25 = 25;
    int v26 = 26;
    int v27 = 27;
    int v28 = 28;
    int v29 = 29;
    int v30 = 30;
    int v31 = 31;
    int v32 = 32;
    int v33 = 33;
    int v34 = 34;
    int v35 = 35;
    int v36 = 36;
    int v37 = 37;
    int v38 = 38;
    int v39 = 39;
    int v40 = 40;
    int v41 = 41;
    int v42 = 42;
    int v43 = 43;
    int v44 = 44;
    int v45 = 45;
    int v46 = 46;
    int v47 = 47;
    int v48 = 48;
    int v49 = 49;
    int v50 = 50;
    int v51 = 51;
    v1 = v51;
}

void test_all_violations() {
    // 4. UNINIT_VAR
    int uninit_var;

    // 5. NO_NEW
    int* ptr = new int;

    // 6. NO_DELETE
    delete ptr;

    // 7. NO_MALLOC_FAMILY
    void* mem = malloc(64);
    free(mem);

    // 8. NO_BANNED_FUNCS
    char buf[16];
    gets(buf);
    sprintf(buf, "%d", 1);
    atoi(buf);

    // 9. NO_ASSIGN_IN_COND
    int val = 0;
    if (val = 1) {
        val = 2;
    }

    // 10. IF_BRACES
    if (val > 0)
        val--;

    // 11. LOOP_BRACES
    while (val < 5)
        val++;

    // 12. NO_CONTINUE
    while (val > 0) {
        val--;
        continue;
    }

    // 13. SWITCH_DEFAULT
    switch (val) {
        case 1:
            break;
    }

    // 14. NO_GOTO
    goto target;
target:

    // 15. MAX_NESTING (> 4 levels)
    if (1) {
        if (1) {
            if (1) {
                if (1) {
                    if (1) {
                        val = 42;
                    }
                }
            }
        }
    }
}
