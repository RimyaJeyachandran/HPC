#include <stdio.h>
#include <omp.h>

// Function to integrate: f(x) = x^2
double f(double x) {
    return x * x;
}

int main() {
    int n, threads;
    double a, b, h, total_area;
    double mid_sum = 0.0;

    printf("Lower limit (a): ");
    scanf("%lf", &a);
    printf("Upper limit (b): ");
    scanf("%lf", &b);
    printf("Number of trapezoids: ");
    scanf("%d", &n);
    printf("Thread count: ");
    scanf("%d", &threads);

    // Step size
    h = (b - a) / n;
    
    // Initial sum: [f(a) + f(b)] / 2
    total_area = (f(a) + f(b)) / 2.0;

    printf("\n--- Calculation Steps ---\n");
    printf("h = %.2f\n", h);
    printf("Initial boundary sum = %.2f\n", total_area);

    // Parallel loop with reduction for the intermediate points
    #pragma omp parallel for num_threads(threads) reduction(+:mid_sum)
    for (int i = 1; i < n; i++) {
        double x_i = a + i * h;
        double y_i = f(x_i);
        
        // Critical block just to keep the print statements from overlapping
        #pragma omp critical
        {
            printf("[Thread %d] i=%d, x=%.2f, f(x)=%.2f\n", omp_get_thread_num(), i, x_i, y_i);
        }
        mid_sum += y_i;
    }

    printf("\nIntermediate sum: %.2f\n", mid_sum);
    
    // Final result: h * [ (f(a)+f(b))/2 + sum(f(xi)) ]
    double final_res = h * (total_area + mid_sum);

    printf("Total sum (boundaries + intermediate): %.2f\n", total_area + mid_sum);
    printf("Final Estimated Integral: %.4f\n", final_res);

    return 0;
}