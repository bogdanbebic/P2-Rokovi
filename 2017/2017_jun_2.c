#include <stdio.h>
#include <stdlib.h>

#define MAX_SCHOOLS 400
#define NAME_LEN    31
#define UNASSIGNED  "NEUPISAN"

#define CHECK_ALLOC(p) if (!(p)) fputs("Neuspesna alokacija", stderr), exit(1)
#define CHECK_FILE(f)  if (!(f)) perror(NULL), exit(2)

typedef struct {
    int  free_spots;
    char name[NAME_LEN];
    // Ne mora se čuvati redni broj jer je sam indeks škole u nizu redni broj
} School;

typedef School SchoolArray[MAX_SCHOOLS];

typedef struct {
    int   id;
    char  name[NAME_LEN];
    float gpa;
    int   wish;
} Student;

typedef struct node {
    Student st;
    struct node *next;
} StudentNode;

int read_schools(SchoolArray schools, FILE *fin)
{
    int i = 0;
    while (fscanf(fin, "%d %[^\n]", &schools[i].free_spots, schools[i].name)==2)
        i++;
    return i;
}

// Ubacuje novi čvor u listu uređeno po želji, pa po proseku, pa po br. prijave
StudentNode *insert_node(StudentNode *head, StudentNode *node)
{
    StudentNode *p = head, *prev = NULL;
    Student st = node->st;
    
    while (p &&
          (st.wish >  p->st.wish
        || st.wish == p->st.wish && st.gpa  < p->st.gpa
        || st.wish == p->st.wish && st.gpa == p->st.gpa && st.id > p->st.id))
    {
        prev = p;
        p = p->next;
    }

    if (prev) prev->next = node;
    else      head = node;
    node->next = p;

    return head;
}

StudentNode *read_students(FILE *fin)
{
    StudentNode *head = NULL;
    Student st;
    
    while (fread(&st, sizeof st, 1, fin)) {
        StudentNode *node = malloc(sizeof *node);
        CHECK_ALLOC(node);
        
        node->st = st;
        node->next = NULL;

        head = insert_node(head, node);
    }

    return head;
}

void free_students(StudentNode *head)
{
    while (head) {
        StudentNode *node = head;
        head = head->next;
        free(node);
    }
}

void sorting_hat(StudentNode *students, SchoolArray schools, int n, FILE *fout)
{
    for (StudentNode *p = students; p; p = p->next) {
        fprintf(fout, "%d %s %.2f ", p->st.id, p->st.name, p->st.gpa);
        
        int i = p->st.wish;
        if (i >= 0 && i < n && schools[i].free_spots > 0) {
            fprintf(fout, "%s\n", schools[i].name);
            schools[i].free_spots--;
        } else {
            fprintf(fout, UNASSIGNED"\n");
        }
    }
}

int main(void)
{
    FILE *f_schools, *f_wishes, *f_admissions;
    CHECK_FILE(f_schools    = fopen("skole.txt", "r"));
    CHECK_FILE(f_wishes     = fopen("zelje.pod", "rb"));
    CHECK_FILE(f_admissions = fopen("upisi.txt", "w"));

    SchoolArray schools;
    int n = read_schools(schools, f_schools);

    StudentNode *students = read_students(f_wishes);

    sorting_hat(students, schools, n, f_admissions);

    free_students(students);
    fclose(f_schools);
    fclose(f_wishes);
    fclose(f_admissions);
}
