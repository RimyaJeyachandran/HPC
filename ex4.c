#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> // Required for offsetof

#define MAX 50
#define TOTAL 4

typedef struct {
    char name[MAX];
    int roll;
    float marks;
    char grade;
} Student;

// Logic to convert numeric marks to letter grades
char get_grade(float m) {
    if (m >= 90) return 'O'; // Based on your output file showing 'O'
    if (m >= 80) return 'A';
    if (m >= 70) return 'B';
    if (m >= 60) return 'C';
    return 'F';
}

int main(int argc, char** argv) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int per_proc = TOTAL / size;

    // --- Create Custom MPI Type ---
    MPI_Datatype student_type;
    int blocks[] = {MAX, 1, 1, 1};
    MPI_Datatype types[] = {MPI_CHAR, MPI_INT, MPI_FLOAT, MPI_CHAR};
    MPI_Aint disp[4];

    disp[0] = offsetof(Student, name);
    disp[1] = offsetof(Student, roll);
    disp[2] = offsetof(Student, marks);
    disp[3] = offsetof(Student, grade);

    MPI_Type_create_struct(4, blocks, disp, types, &student_type);
    MPI_Type_commit(&student_type);

    Student *full_list = NULL;
    Student *my_part = malloc(per_proc * sizeof(Student));

    if (rank == 0) {
        full_list = malloc(TOTAL * sizeof(Student));
        FILE *fin = fopen("input_students.txt", "r");
        
        if (!fin) {
            // Fallback if file is missing
            for(int i=0; i<TOTAL; i++) {
                sprintf(full_list[i].name, "Stud%d", i);
                full_list[i].roll = 100+i;
                full_list[i].marks = 85.0;
            }
        } else {
            for (int i = 0; i < TOTAL; i++) {
                fscanf(fin, "%s %d %f", full_list[i].name, &full_list[i].roll, &full_list[i].marks);
            }
            fclose(fin);
        }
    }

    // Move data from Root to everyone
    MPI_Scatter(full_list, per_proc, student_type, my_part, per_proc, student_type, 0, MPI_COMM_WORLD);

    // Calculate grades locally
    for (int i = 0; i < per_proc; i++) {
        my_part[i].grade = get_grade(my_part[i].marks);
        printf("[P%d] Processed %s: %c\n", rank, my_part[i].name, my_part[i].grade);
    }

    // Pull results back to Root
    MPI_Gather(my_part, per_proc, student_type, full_list, per_proc, student_type, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        FILE *fout = fopen("grades.txt", "w");
        fprintf(fout, "Name\tRollNo\tMarks\tGrade\n-------------------------------------\n");
        for (int i = 0; i < TOTAL; i++) {
            fprintf(fout, "%s\t%d\t%.2f\t%c\n", full_list[i].name, full_list[i].roll, full_list[i].marks, full_list[i].grade);
        }
        fclose(fout);
        printf("\nDone. Results saved to grades.txt\n");
        free(full_list);
    }

    free(my_part);
    MPI_Type_free(&student_type);
    MPI_Finalize();
    return 0;
}