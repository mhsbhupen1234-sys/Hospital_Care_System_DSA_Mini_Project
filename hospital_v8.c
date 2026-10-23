#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================ PATIENT RECORD ================ */

struct Patient {
    int id;
    char name[50];
    int age;
    char gender[15];
    char disease[50];
    struct Patient *next;
};

/* ================ REGULAR QUEUE ================ */

struct QueueNode {
    int patientID;
    char patientName[50];
    struct QueueNode *next;
};

/* ================ EMERGENCY QUEUE ================ */

struct EmergencyNode {
    int patientID;
    char patientName[50];
    int priority;
    struct EmergencyNode *next;
};

/* ================ APPOINTMENT LIST ================ */

struct Appointment {
    int appointmentID;
    int patientID;
    char patientName[50];
    char date[11];       // DD-MM-YYYY
    char time[9];        // HH:MM AM/PM
    struct Appointment *next;
};

/* ================ GLOBAL POINTERS ================ */

struct Patient *head = NULL;

struct QueueNode *front = NULL;
struct QueueNode *rear = NULL;

struct EmergencyNode *emergencyFront = NULL;

struct Appointment *appointmentHead = NULL;

int nextPatientID = 1001;
int nextAppointmentID = 1;

/* Presentation helper declarations (defined below). */
void printDivider(void);
void printSectionTitle(const char *title);

/* ================ INPUT HELPER ================ */

void clearInputBuffer(void) {
    int ch;

    while ((ch = getchar()) != '\n' && ch != EOF) {
    }
}

/* ================ FIND PATIENT ================ */

struct Patient *findPatient(int id) {
    struct Patient *temp = head;

    while (temp != NULL) {
        if (temp->id == id) {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}


/* ================ FILE HANDLING (V7) ================ */
/* Data files are created in the program's current working directory:
   patients.txt, appointments.txt, regular_queue.txt, emergency_queue.txt.
   Each record is written as a text line using | as a separator. */

int savePatientData(void) {
    FILE *file = fopen("patients.txt", "w");
    if (file == NULL) return 0;

    struct Patient *temp = head;
    while (temp != NULL) {
        fprintf(file, "%d|%s|%d|%s|%s\n",
                temp->id, temp->name, temp->age,
                temp->gender, temp->disease);
        temp = temp->next;
    }

    int ok = (fclose(file) == 0);
    return ok;
}

int saveAppointmentData(void) {
    FILE *file = fopen("appointments.txt", "w");
    if (file == NULL) return 0;

    struct Appointment *temp = appointmentHead;
    while (temp != NULL) {
        fprintf(file, "%d|%d|%s|%s|%s\n",
                temp->appointmentID, temp->patientID,
                temp->patientName, temp->date, temp->time);
        temp = temp->next;
    }

    int ok = (fclose(file) == 0);
    return ok;
}

int saveRegularQueueData(void) {
    FILE *file = fopen("regular_queue.txt", "w");
    if (file == NULL) return 0;

    struct QueueNode *temp = front;
    while (temp != NULL) {
        fprintf(file, "%d|%s\n", temp->patientID, temp->patientName);
        temp = temp->next;
    }

    int ok = (fclose(file) == 0);
    return ok;
}

int saveEmergencyQueueData(void) {
    FILE *file = fopen("emergency_queue.txt", "w");
    if (file == NULL) return 0;

    struct EmergencyNode *temp = emergencyFront;
    while (temp != NULL) {
        fprintf(file, "%d|%s|%d\n",
                temp->patientID, temp->patientName, temp->priority);
        temp = temp->next;
    }

    int ok = (fclose(file) == 0);
    return ok;
}

int saveAllData(void) {
    int patientsOK = savePatientData();
    int appointmentsOK = saveAppointmentData();
    int regularQueueOK = saveRegularQueueData();
    int emergencyQueueOK = saveEmergencyQueueData();

    return patientsOK && appointmentsOK &&
           regularQueueOK && emergencyQueueOK;
}

void autoSaveData(void) {
    if (!saveAllData()) {
        printf("\nWarning: Could not save one or more data files.\n");
    }
}

int loadPatientData(void) {
    FILE *file = fopen("patients.txt", "r");
    if (file == NULL) return 0;

    char line[256];
    int count = 0;
    int maxID = 1000;

    while (fgets(line, sizeof(line), file) != NULL) {
        struct Patient *patient = malloc(sizeof(struct Patient));
        if (patient == NULL) break;

        int fields = sscanf(line, "%d|%49[^|]|%d|%14[^|]|%49[^\r\n]",
                            &patient->id, patient->name, &patient->age,
                            patient->gender, patient->disease);

        if (fields != 5) {
            free(patient);
            continue;
        }

        patient->next = NULL;

        struct Patient **link = &head;
        while (*link != NULL) link = &((*link)->next);
        *link = patient;

        if (patient->id > maxID) maxID = patient->id;
        count++;
    }

    fclose(file);
    nextPatientID = maxID + 1;
    return count;
}

int loadAppointmentData(void) {
    FILE *file = fopen("appointments.txt", "r");
    if (file == NULL) return 0;

    char line[256];
    int count = 0;
    int maxID = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        struct Appointment *appointment =
            malloc(sizeof(struct Appointment));
        if (appointment == NULL) break;

        int fields = sscanf(line, "%d|%d|%49[^|]|%10[^|]|%8[^\r\n]",
                            &appointment->appointmentID,
                            &appointment->patientID,
                            appointment->patientName,
                            appointment->date,
                            appointment->time);

        if (fields != 5) {
            free(appointment);
            continue;
        }

        appointment->next = NULL;

        struct Appointment **link = &appointmentHead;
        while (*link != NULL) link = &((*link)->next);
        *link = appointment;

        if (appointment->appointmentID > maxID)
            maxID = appointment->appointmentID;
        count++;
    }

    fclose(file);
    nextAppointmentID = maxID + 1;
    return count;
}

int loadRegularQueueData(void) {
    FILE *file = fopen("regular_queue.txt", "r");
    if (file == NULL) return 0;

    char line[128];
    int count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        int id;
        char savedName[50];

        if (sscanf(line, "%d|%49[^\r\n]", &id, savedName) != 2)
            continue;

        struct Patient *patient = findPatient(id);
        if (patient == NULL) continue;

        struct QueueNode *node = malloc(sizeof(struct QueueNode));
        if (node == NULL) break;

        node->patientID = id;
        strcpy(node->patientName, patient->name);
        node->next = NULL;

        if (front == NULL) front = rear = node;
        else {
            rear->next = node;
            rear = node;
        }
        count++;
    }

    fclose(file);
    return count;
}

int loadEmergencyQueueData(void) {
    FILE *file = fopen("emergency_queue.txt", "r");
    if (file == NULL) return 0;

    char line[128];
    int count = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        int id, priority;
        char savedName[50];

        if (sscanf(line, "%d|%49[^|]|%d", &id, savedName, &priority) != 3)
            continue;
        if (priority < 1 || priority > 3) continue;

        struct Patient *patient = findPatient(id);
        if (patient == NULL) continue;

        struct EmergencyNode *node =
            malloc(sizeof(struct EmergencyNode));
        if (node == NULL) break;

        node->patientID = id;
        strcpy(node->patientName, patient->name);
        node->priority = priority;
        node->next = NULL;

        /* File order already contains the priority-queue order. */
        if (emergencyFront == NULL) {
            emergencyFront = node;
        } else {
            struct EmergencyNode *tail = emergencyFront;
            while (tail->next != NULL) tail = tail->next;
            tail->next = node;
        }
        count++;
    }

    fclose(file);
    return count;
}

void loadAllData(void) {
    int patients = loadPatientData();
    int appointments = loadAppointmentData();
    int regular = loadRegularQueueData();
    int emergency = loadEmergencyQueueData();

    if (patients + appointments + regular + emergency > 0) {
        printf("\nExisting data loaded successfully:\n");
        printf("Patients: %d | Appointments: %d | Regular queue: %d | Emergency queue: %d\n",
               patients, appointments, regular, emergency);
    }
}


/* ================ REGISTER PATIENT ================ */

void registerPatient(void) {
    struct Patient *newPatient = malloc(sizeof(struct Patient));

    if (newPatient == NULL) {
        printf("\nMemory allocation failed!\n");
        return;
    }

    printSectionTitle("PATIENT REGISTRATION");

    printf("Enter patient name: ");

    if (scanf(" %49[^\n]", newPatient->name) != 1) {
        printf("Invalid name!\n");
        free(newPatient);
        return;
    }

    printf("Enter age: ");

    if (scanf("%d", &newPatient->age) != 1 ||
        newPatient->age < 0 || newPatient->age > 130) {

        printf("Invalid age!\n");
        clearInputBuffer();
        free(newPatient);
        return;
    }

    printf("Enter gender: ");

    if (scanf(" %14s", newPatient->gender) != 1) {
        printf("Invalid gender!\n");
        free(newPatient);
        return;
    }

    printf("Enter condition or reason for visit: ");

    if (scanf(" %49[^\n]", newPatient->disease) != 1) {
        printf("Invalid condition!\n");
        free(newPatient);
        return;
    }

    newPatient->id = nextPatientID++;
    newPatient->next = NULL;

    if (head == NULL) {
        head = newPatient;
    } else {
        struct Patient *temp = head;

        while (temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = newPatient;
    }

    printf("\nPatient registered successfully!\n");
    printf("Patient ID: %d\n", newPatient->id);
    autoSaveData();
}

/* ================ DISPLAY PATIENTS ================ */

void displayPatients(void) {
    struct Patient *temp = head;

    if (head == NULL) {
        printf("\nNo patient records found.\n");
        return;
    }

    printSectionTitle("REGISTERED PATIENTS");

    while (temp != NULL) {
        printf("\nPatient ID : %d\n", temp->id);
        printf("Name       : %s\n", temp->name);
        printf("Age        : %d\n", temp->age);
        printf("Gender     : %s\n", temp->gender);
        printf("Condition  : %s\n", temp->disease);
        printf("-----------------------------------------\n");

        temp = temp->next;
    }
}

/* ================ SEARCH PATIENT ================ */

void searchPatient(void) {
    int id;

    printf("\nEnter Patient ID: ");

    if (scanf("%d", &id) != 1) {
        printf("Invalid ID!\n");
        clearInputBuffer();
        return;
    }

    struct Patient *patient = findPatient(id);

    if (patient == NULL) {
        printf("\nPatient not found.\n");
        return;
    }

    printf("\nPatient Found!\n");
    printf("ID        : %d\n", patient->id);
    printf("Name      : %s\n", patient->name);
    printf("Age       : %d\n", patient->age);
    printf("Gender    : %s\n", patient->gender);
    printf("Condition : %s\n", patient->disease);
}

/* ================ REGULAR QUEUE ================ */

void addToQueue(void) {
    int id;

    printf("\nEnter registered Patient ID: ");

    if (scanf("%d", &id) != 1) {
        printf("Invalid ID!\n");
        clearInputBuffer();
        return;
    }

    struct Patient *patient = findPatient(id);

    if (patient == NULL) {
        printf("\nRegister the patient first.\n");
        return;
    }

    struct QueueNode *check = front;

    while (check != NULL) {
        if (check->patientID == id) {
            printf("\nPatient is already in the regular queue.\n");
            return;
        }

        check = check->next;
    }

    struct QueueNode *newNode = malloc(sizeof(struct QueueNode));

    if (newNode == NULL) {
        printf("\nMemory allocation failed!\n");
        return;
    }

    newNode->patientID = patient->id;
    strcpy(newNode->patientName, patient->name);
    newNode->next = NULL;

    if (front == NULL) {
        front = rear = newNode;
    } else {
        rear->next = newNode;
        rear = newNode;
    }

    printf("\nPatient added to regular queue.\n");
    autoSaveData();
}

void displayQueue(void) {
    struct QueueNode *temp = front;

    if (front == NULL) {
        printf("\nRegular queue is empty.\n");
        return;
    }

    printSectionTitle("REGULAR WAITING QUEUE - FIFO ORDER");

    while (temp != NULL) {
        printf("ID: %d | Name: %s\n",
               temp->patientID, temp->patientName);

        temp = temp->next;
    }
}

void callNextPatient(void) {
    if (front == NULL) {
        printf("\nNo patients in the regular queue.\n");
        return;
    }

    struct QueueNode *temp = front;

    printf("\nNext regular patient:\n");
    printf("ID: %d\n", temp->patientID);
    printf("Name: %s\n", temp->patientName);

    front = front->next;

    if (front == NULL) {
        rear = NULL;
    }

    free(temp);
    autoSaveData();
}

/* ================ EMERGENCY PRIORITY QUEUE ================ */

void addEmergencyPatient(void) {
    int id;
    int priority;

    printf("\nEnter registered Patient ID: ");

    if (scanf("%d", &id) != 1) {
        printf("Invalid ID!\n");
        clearInputBuffer();
        return;
    }

    struct Patient *patient = findPatient(id);

    if (patient == NULL) {
        printf("\nRegister the patient first.\n");
        return;
    }

    struct EmergencyNode *check = emergencyFront;

    while (check != NULL) {
        if (check->patientID == id) {
            printf("\nPatient is already in the emergency queue.\n");
            return;
        }

        check = check->next;
    }

    printf("\nSelect simulated priority:\n");
    printf("1. Critical priority\n");
    printf("2. High priority\n");
    printf("3. Medium priority\n");
    printf("Enter priority: ");

    if (scanf("%d", &priority) != 1) {
        printf("Invalid priority!\n");
        clearInputBuffer();
        return;
    }

    if (priority < 1 || priority > 3) {
        printf("\nInvalid priority! Choose 1, 2, or 3.\n");
        return;
    }

    struct EmergencyNode *newNode =
        malloc(sizeof(struct EmergencyNode));

    if (newNode == NULL) {
        printf("\nMemory allocation failed!\n");
        return;
    }

    newNode->patientID = patient->id;
    strcpy(newNode->patientName, patient->name);
    newNode->priority = priority;
    newNode->next = NULL;

    if (emergencyFront == NULL ||
        priority < emergencyFront->priority) {

        newNode->next = emergencyFront;
        emergencyFront = newNode;

    } else {
        struct EmergencyNode *temp = emergencyFront;

        while (temp->next != NULL &&
               temp->next->priority <= priority) {

            temp = temp->next;
        }

        newNode->next = temp->next;
        temp->next = newNode;
    }

    printf("\nPatient added to emergency priority queue.\n");
    autoSaveData();
}

void displayEmergencyQueue(void) {
    struct EmergencyNode *temp = emergencyFront;

    if (emergencyFront == NULL) {
        printf("\nEmergency queue is empty.\n");
        return;
    }

    printSectionTitle("EMERGENCY PRIORITY QUEUE");

    while (temp != NULL) {
        printf("ID: %d | Name: %s | Priority: %d\n",
               temp->patientID,
               temp->patientName,
               temp->priority);

        temp = temp->next;
    }
}

void processEmergencyPatient(void) {
    if (emergencyFront == NULL) {
        printf("\nNo patients in the emergency queue.\n");
        return;
    }

    struct EmergencyNode *temp = emergencyFront;

    printSectionTitle("NEXT EMERGENCY PATIENT");
    printf("Patient ID: %d\n", temp->patientID);
    printf("Patient Name: %s\n", temp->patientName);
    printf("Priority: %d\n", temp->priority);

    emergencyFront = emergencyFront->next;

    free(temp);

    printf("\nPatient removed from the emergency queue.\n");
    autoSaveData();
}

/* ================ APPOINTMENT FUNCTIONS ================ */

/* Find appointment by appointment ID */
struct Appointment *findAppointment(int appointmentID) {
    struct Appointment *temp = appointmentHead;

    while (temp != NULL) {
        if (temp->appointmentID == appointmentID) {
            return temp;
        }

        temp = temp->next;
    }

    return NULL;
}

/* Schedule appointment */
void scheduleAppointment(void) {
    int patientID;
    struct Patient *patient;

    printf("\nEnter registered Patient ID: ");

    if (scanf("%d", &patientID) != 1) {
        printf("Invalid Patient ID!\n");
        clearInputBuffer();
        return;
    }

    patient = findPatient(patientID);

    if (patient == NULL) {
        printf("\nPatient not found. Register the patient first.\n");
        return;
    }

    /* Prevent multiple active appointments for one patient */
    struct Appointment *check = appointmentHead;

    while (check != NULL) {
        if (check->patientID == patientID) {
            printf("\nThis patient already has an active appointment.\n");
            printf("Appointment ID: %d\n", check->appointmentID);
            return;
        }

        check = check->next;
    }

    struct Appointment *newAppointment =
        malloc(sizeof(struct Appointment));

    if (newAppointment == NULL) {
        printf("\nMemory allocation failed!\n");
        return;
    }

    printf("Enter appointment date (DD-MM-YYYY): ");

    if (scanf("%10s", newAppointment->date) != 1) {
        printf("Invalid date input!\n");
        free(newAppointment);
        return;
    }

    printf("Enter appointment time (e.g. 10:30AM): ");

    if (scanf("%8s", newAppointment->time) != 1) {
        printf("Invalid time input!\n");
        free(newAppointment);
        return;
    }

    newAppointment->appointmentID = nextAppointmentID++;
    newAppointment->patientID = patient->id;

    strcpy(newAppointment->patientName, patient->name);

    newAppointment->next = NULL;

    if (appointmentHead == NULL) {
        appointmentHead = newAppointment;
    } else {
        struct Appointment *temp = appointmentHead;

        while (temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = newAppointment;
    }

    printf("\nAppointment scheduled successfully!\n");
    printf("Appointment ID: %d\n", newAppointment->appointmentID);
    printf("Patient Name: %s\n", newAppointment->patientName);
    autoSaveData();
}

/* Display appointments */
void displayAppointments(void) {
    struct Appointment *temp = appointmentHead;

    if (appointmentHead == NULL) {
        printf("\nNo appointments scheduled.\n");
        return;
    }

    printSectionTitle("APPOINTMENT LIST");

    while (temp != NULL) {
        printf("\nAppointment ID : %d\n", temp->appointmentID);
        printf("Patient ID     : %d\n", temp->patientID);
        printf("Patient Name   : %s\n", temp->patientName);
        printf("Date           : %s\n", temp->date);
        printf("Time           : %s\n", temp->time);
        printf("--------------------------------------\n");

        temp = temp->next;
    }
}

/* Search appointment using patient ID */
void searchAppointment(void) {
    int patientID;
    int found = 0;

    printf("\nEnter Patient ID: ");

    if (scanf("%d", &patientID) != 1) {
        printf("Invalid Patient ID!\n");
        clearInputBuffer();
        return;
    }

    struct Appointment *temp = appointmentHead;

    while (temp != NULL) {
        if (temp->patientID == patientID) {
            printf("\nAppointment Found!\n");
            printf("Appointment ID: %d\n", temp->appointmentID);
            printf("Patient Name: %s\n", temp->patientName);
            printf("Date: %s\n", temp->date);
            printf("Time: %s\n", temp->time);

            found = 1;
            break;
        }

        temp = temp->next;
    }

    if (!found) {
        printf("\nNo appointment found for this patient.\n");
    }
}

/* Cancel appointment */
void cancelAppointment(void) {
    int appointmentID;

    printf("\nEnter Appointment ID to cancel: ");

    if (scanf("%d", &appointmentID) != 1) {
        printf("Invalid Appointment ID!\n");
        clearInputBuffer();
        return;
    }

    struct Appointment *temp = appointmentHead;
    struct Appointment *previous = NULL;

    while (temp != NULL &&
           temp->appointmentID != appointmentID) {

        previous = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("\nAppointment not found.\n");
        return;
    }

    if (previous == NULL) {
        appointmentHead = temp->next;
    } else {
        previous->next = temp->next;
    }

    free(temp);

    printf("\nAppointment cancelled successfully!\n");
    autoSaveData();
}


/* ================ SORT PATIENT RECORDS ================ */

/*
   Sort linked-list nodes by relinking them, rather than swapping
   the patient data. mode: 1 = ID, 2 = name, 3 = age.
*/
int patientOutOfOrder(struct Patient *a, struct Patient *b, int mode) {
    if (mode == 1) {
        return a->id > b->id;
    } else if (mode == 2) {
        return strcmp(a->name, b->name) > 0;
    } else {
        return a->age > b->age;
    }
}

void sortPatients(int mode) {
    if (head == NULL || head->next == NULL) {
        printf("\nNot enough patient records to sort.\n");
        return;
    }

    int swapped;
    do {
        swapped = 0;
        struct Patient **link = &head;

        while (*link != NULL && (*link)->next != NULL) {
            struct Patient *a = *link;
            struct Patient *b = a->next;

            if (patientOutOfOrder(a, b, mode)) {
                /* Swap adjacent nodes by changing links. */
                a->next = b->next;
                b->next = a;
                *link = b;
                swapped = 1;
            }

            /* Move to the link that points to node a. */
            link = &((*link)->next);
        }
    } while (swapped);

    if (mode == 1) {
        printf("\nPatient records sorted by Patient ID (ascending).\n");
    } else if (mode == 2) {
        printf("\nPatient records sorted by Name (A-Z).\n");
    } else {
        printf("\nPatient records sorted by Age (ascending).\n");
    }

    displayPatients();
    autoSaveData();
}

void sortingMenu(void) {
    int sortChoice;

    do {
        printSectionTitle("SORT PATIENT RECORDS");
        printf("1. Sort by Patient ID (ascending)\n");
        printf("2. Sort by Patient Name (A-Z)\n");
        printf("3. Sort by Age (ascending)\n");
        printf("0. Return to Main Menu\n");
        printf("Enter your choice: ");

        if (scanf("%d", &sortChoice) != 1) {
            printf("\nInvalid input! Enter a number.\n");
            clearInputBuffer();
            continue;
        }

        switch (sortChoice) {
            case 1:
                sortPatients(1);
                break;
            case 2:
                sortPatients(2);
                break;
            case 3:
                sortPatients(3);
                break;
            case 0:
                break;
            default:
                printf("\nInvalid sorting choice! Try again.\n");
        }
    } while (sortChoice != 0);
}



/* ================ PRESENTATION AND DASHBOARD (V8) ================ */

void printDivider(void) {
    printf("+----------------------------------------------------------+\n");
}

void printSectionTitle(const char *title) {
    printf("\n");
    printDivider();
    printf("| %-56s |\n", title);
    printDivider();
}

int countPatients(void) {
    int count = 0;
    struct Patient *temp = head;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

int countRegularQueue(void) {
    int count = 0;
    struct QueueNode *temp = front;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

int countEmergencyQueue(void) {
    int count = 0;
    struct EmergencyNode *temp = emergencyFront;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

int countAppointments(void) {
    int count = 0;
    struct Appointment *temp = appointmentHead;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

void showDashboard(void) {
    int critical = 0, high = 0, medium = 0;
    struct EmergencyNode *temp = emergencyFront;

    while (temp != NULL) {
        if (temp->priority == 1) critical++;
        else if (temp->priority == 2) high++;
        else if (temp->priority == 3) medium++;
        temp = temp->next;
    }

    printSectionTitle("HOSPITAL OPERATIONS DASHBOARD");
    printf("  Registered patients       : %d\n", countPatients());
    printf("  Regular waiting queue     : %d\n", countRegularQueue());
    printf("  Emergency queue           : %d\n", countEmergencyQueue());
    printf("      - Critical priority   : %d\n", critical);
    printf("      - High priority       : %d\n", high);
    printf("      - Medium priority     : %d\n", medium);
    printf("  Active appointments       : %d\n", countAppointments());
    printf("  Next patient ID           : %d\n", nextPatientID);
    printDivider();
    printf("  Data files are updated automatically after changes.\n");
}

void showProjectInfo(void) {
    printSectionTitle("PROJECT OVERVIEW AND DSA CONCEPTS");
    printf("  Project : Hospital Patient Management and Emergency Queue\n");
    printf("            System Using Data Structures in C\n\n");
    printf("  DATA STRUCTURE / CONCEPT       APPLICATION IN PROJECT\n");
    printDivider();
    printf("  Singly linked list             Dynamic patient records\n");
    printf("  FIFO linked queue              Regular waiting queue\n");
    printf("  Priority queue                 Emergency patient ordering\n");
    printf("  Linked list + Bubble Sort      Sort by ID, name, or age\n");
    printf("  File handling                  Save and reload records\n");
    printf("  malloc() and free()            Dynamic memory management\n");
    printDivider();
    printf("  NOTE: Emergency priorities are a simplified academic\n");
    printf("  simulation and are not a substitute for clinical triage.\n");
}

void showMainHeader(void) {
    printf("\n");
    printf("+==========================================================+\n");
    printf("|                    HOSPITAL CARE SYSTEM                  |\n");
    printf("|        PATIENT MANAGEMENT & EMERGENCY QUEUE              |\n");
    printf("+----------------------------------------------------------+\n");
    printf("|                 DSA MINI PROJECT IN C                    |\n");
    printf("+==========================================================+\n");
}


/* ================ FREE MEMORY ================ */

void freePatientRecords(void) {
    struct Patient *temp;

    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void freeRegularQueue(void) {
    struct QueueNode *temp;

    while (front != NULL) {
        temp = front;
        front = front->next;
        free(temp);
    }

    rear = NULL;
}

void freeEmergencyQueue(void) {
    struct EmergencyNode *temp;

    while (emergencyFront != NULL) {
        temp = emergencyFront;
        emergencyFront = emergencyFront->next;
        free(temp);
    }
}

void freeAppointments(void) {
    struct Appointment *temp;

    while (appointmentHead != NULL) {
        temp = appointmentHead;
        appointmentHead = appointmentHead->next;
        free(temp);
    }
}

/* ================ MAIN FUNCTION ================ */

int main(void) {
    int choice;

    loadAllData();

    do {
        showMainHeader();
        printf("\n  PATIENT MANAGEMENT\n");
        printf("  1. Register New Patient       2. View All Patients\n");
        printf("  3. Search Patient             14. Sort Patient Records\n");
        printf("\n  QUEUE OPERATIONS\n");
        printf("  4. Add to Regular Queue       5. Display Regular Queue\n");
        printf("  6. Call Next Regular Patient\n");
        printf("\n  EMERGENCY SERVICES\n");
        printf("  7. Add Emergency Patient      8. Display Emergency Queue\n");
        printf("  9. Process Next Emergency Patient\n");
        printf("\n  APPOINTMENT MANAGEMENT\n");
        printf(" 10. Schedule Appointment      11. View Appointments\n");
        printf(" 12. Search Appointment        13. Cancel Appointment\n");
        printf("\n  REPORTS AND SYSTEM\n");
        printf(" 15. View Operations Dashboard 16. Project / DSA Info\n");
        printf("  0. Save and Exit\n");
        printDivider();
        printf("  Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("\n  [!] Invalid input. Please enter a menu number.\n");
            clearInputBuffer();
            continue;
        }

        switch (choice) {
            case 1:  registerPatient(); break;
            case 2:  displayPatients(); break;
            case 3:  searchPatient(); break;
            case 4:  addToQueue(); break;
            case 5:  displayQueue(); break;
            case 6:  callNextPatient(); break;
            case 7:  addEmergencyPatient(); break;
            case 8:  displayEmergencyQueue(); break;
            case 9:  processEmergencyPatient(); break;
            case 10: scheduleAppointment(); break;
            case 11: displayAppointments(); break;
            case 12: searchAppointment(); break;
            case 13: cancelAppointment(); break;
            case 14: sortingMenu(); break;
            case 15: showDashboard(); break;
            case 16: showProjectInfo(); break;

            case 0:
                if (saveAllData()) {
                    printf("\n  [SUCCESS] All data saved to text files.\n");
                } else {
                    printf("\n  [WARNING] One or more data files could not be saved.\n");
                }
                freeRegularQueue();
                freeEmergencyQueue();
                freeAppointments();
                freePatientRecords();
                printf("\n  Thank you for using Hospital Care System.\n");
                printf("  Program closed safely.\n\n");
                break;

            default:
                printf("\n  [!] Invalid choice. Select an option from the menu.\n");
        }

    } while (choice != 0);

    return 0;
}

