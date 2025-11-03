#include "mail_system.h"
#include <signal.h>

static SharedMemoryData* g_shm_ptr = NULL;

void signal_handler(int sig) {
    printf("\nReceived signal %d, cleaning up...\n", sig);
    if (g_shm_ptr != NULL) {
        save_users_to_file(g_shm_ptr);
        save_emails_to_file(g_shm_ptr);
        detach_shared_memory(g_shm_ptr);
    }
    cleanup_shared_memory();
    exit(0);
}

void clear_screen() {
    system("clear");
}

void pause_system() {
    printf("\nPress Enter to continue...");
    getchar();
}

int get_user_choice() {
    int choice;
    char buffer[100];
    
    while (1) {
        printf("Enter your choice: ");
        fflush(stdout);
        
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = 0;
            
            if (strlen(buffer) == 0) {
                if (feof(stdin)) {
                    return 10;
                }
                printf("Please enter a number.\n");
                continue;
            }
            
            char* endptr;
            choice = strtol(buffer, &endptr, 10);
            
            if (*endptr == '\0') {
                return choice;
            } else {
                printf("Invalid input! Please enter a valid number.\n");
                continue;
            }
        } else {
            if (feof(stdin)) {
                printf("\nEnd of input reached. Exiting...\n");
                return 10;
            } else {
                printf("Error reading input!\n");
                return 10;
            }
        }
    }
}

void display_menu() {
    printf("\n" "===============================================\n");
    printf("          MAIL SYSTEM - USER MENU\n");
    printf("===============================================\n");
    printf("1.  Compose New Email\n");
    printf("2.  View Sent Emails\n");
    printf("3.  View Received Emails\n");
    printf("4.  Search My Emails\n");
    printf("5.  Update Profile\n");
    printf("6.  View Shared Memory (DEBUG)\n");
    printf("7.  Logout\n");
    printf("===============================================\n");
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    printf("==============================================\n");
    printf("     MAIL SYSTEM WITH SHARED MEMORY IPC\n");
    printf("==============================================\n");
    printf("Initializing system...\n");
    
    g_shm_ptr = attach_shared_memory();
    if (g_shm_ptr == NULL) {
        printf("Failed to attach to shared memory!\n");
        return 1;
    }
    
    init_shared_memory(g_shm_ptr);
    
    printf("System initialized successfully!\n");
    
    if (!handle_authentication(g_shm_ptr)) {
        printf("Authentication failed or user chose to exit.\n");
        detach_shared_memory(g_shm_ptr);
        cleanup_shared_memory();
        return 0;
    }
    
    int choice;
    
    do {
        clear_screen();
        display_menu();
        
        User* current_user = read_user(g_shm_ptr, get_current_user_id());
        if (current_user) {
            printf("Logged in as: %s <%s>\n", current_user->name, current_user->email);
        }
        
        choice = get_user_choice();
        
        switch (choice) {
            case 1:
                compose_mail(g_shm_ptr);
                pause_system();
                break;
            case 2:
                view_sent_mails(g_shm_ptr);
                pause_system();
                break;
            case 3:
                view_received_mails(g_shm_ptr);
                pause_system();
                break;
            case 4:
                search_emails(g_shm_ptr);
                pause_system();
                break;
            case 5:
                edit_user(g_shm_ptr);
                pause_system();
                break;
            case 6:
                display_shared_memory_info(g_shm_ptr);
                pause_system();
                break;
            case 7:
                printf("Logging out...\n");
                logout_user();
                save_users_to_file(g_shm_ptr);
                save_emails_to_file(g_shm_ptr);
                break;
            default:
                printf("Invalid choice! Please try again.\n");
                pause_system();
        }
    } while (choice != 7);
    
    detach_shared_memory(g_shm_ptr);
    cleanup_shared_memory();
    
    printf("Thank you for using Mail System!\n");
    return 0;
}