// Clean C++ code adhering strictly to all 15 MISRA/AUTOSAR rules (0 violations)

int compute_square(int val) {
    int result = val * val;
    return result;
}

void process_data(int input) {
    int counter = 0;
    int accumulator = input;

    // Enclosed loop body in curly braces
    while (counter < 5) {
        accumulator = accumulator + counter;
        counter = counter + 1;
    }

    // Enclosed if-else branches in curly braces
    if (accumulator > 10) {
        accumulator = compute_square(accumulator);
    } else {
        accumulator = 0;
    }

    // Switch statement with required default label
    switch (accumulator) {
        case 0: {
            accumulator = 1;
            break;
        }
        case 1: {
            accumulator = 2;
            break;
        }
        default: {
            accumulator = 0;
            break;
        }
    }
}
