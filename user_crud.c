#include "mail_system.h"

// Global variable to track logged-in user
static int g_current_user_id = -1;

// Authentication helper functions
int is_user_logged_in() {
    return g_current_user_id > 0;
}

int get_current_user_id() {
    return g_current_user_id;
}

void logout_user() {
    g_current_user_id = -1;
    printf("Logged out successfully!\n");
}

User* verify_user_credentials(SharedMemoryData* shm_ptr, const char* email, const char* password) {
    if (shm_ptr == NULL || email == NULL || password == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_USERS; i++) {
        if (shm_ptr->users[i].is_active && 
            strcmp(shm_ptr->users[i].email, email) == 0 &&
            strcmp(shm_ptr->users[i].password, password) == 0) {
            return &shm_ptr->users[i];
        }
    }
    
    return NULL;
}

void show_login_menu() {
    printf("\n" "===============================================\n");
    printf("      MAIL SYSTEM - LOGIN/REGISTER\n");
    printf("===============================================\n");
    printf("1. Login\n");
    printf("2. Register New Account\n");
    printf("3. Exit\n");
    printf("===============================================\n");
}

int handle_authentication(SharedMemoryData* shm_ptr) {
    int choice;
    
    while (1) {
        clear_screen();
        show_login_menu();
        choice = get_user_choice();
        
        switch (choice) {
            case 1: {
                // Login
                char email[MAX_EMAIL_LENGTH];
                char password[MAX_PASSWORD_LENGTH];
                
                printf("\n=== LOGIN ===\n");
                printf("Email: ");
                fgets(email, sizeof(email), stdin);
                email[strcspn(email, "\n")] = 0;
                
                printf("Password: ");
                fgets(password, sizeof(password), stdin);
                password[strcspn(password, "\n")] = 0;
                
                User* user = verify_user_credentials(shm_ptr, email, password);
                if (user != NULL) {
                    g_current_user_id = user->user_id;
                    printf("\nLogin successful! Welcome, %s!\n", user->name);
                    pause_system();
                    return 1;
                } else {
                    printf("\nInvalid email or password!\n");
                    pause_system();
                }
                break;
            }
            case 2: {
                // Register
                register_user(shm_ptr);
                pause_system();
                break;
            }
            case 3:
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
                pause_system();
        }
    }
}

int create_user(SharedMemoryData* shm_ptr, const char* name, const char* email, const char* password, int age) {
    if (shm_ptr == NULL || name == NULL || email == NULL || password == NULL) {
        printf("Error: Invalid parameters\n");
        return -1;
    }
    
    if (shm_ptr->control.user_count >= MAX_USERS) {
        printf("Error: Maximum number of users reached\n");
        return -1;
    }
    
    if (find_user_by_email(shm_ptr, email) != NULL) {
        printf("Error: Email already exists\n");
        return -1;
    }
    
    int index = -1;
    for (int i = 0; i < MAX_USERS; i++) {
        if (!shm_ptr->users[i].is_active) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        printf("Error: No available slot for new user\n");
        return -1;
    }
    
    User* new_user = &shm_ptr->users[index];
    new_user->user_id = shm_ptr->control.next_user_id++;
    strncpy(new_user->name, name, MAX_NAME_LENGTH - 1);
    new_user->name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(new_user->email, email, MAX_EMAIL_LENGTH - 1);
    new_user->email[MAX_EMAIL_LENGTH - 1] = '\0';
    strncpy(new_user->password, password, MAX_PASSWORD_LENGTH - 1);
    new_user->password[MAX_PASSWORD_LENGTH - 1] = '\0';
    new_user->age = age;
    new_user->is_active = 1;
    new_user->created_at = time(NULL);
    
    shm_ptr->control.user_count++;
    
    printf("User created successfully with ID: %d\n", new_user->user_id);
    return new_user->user_id;
}

User* read_user(SharedMemoryData* shm_ptr, int user_id) {
    if (shm_ptr == NULL || user_id <= 0) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_USERS; i++) {
        if (shm_ptr->users[i].is_active && shm_ptr->users[i].user_id == user_id) {
            return &shm_ptr->users[i];
        }
    }
    
    return NULL;
}

User* find_user_by_email(SharedMemoryData* shm_ptr, const char* email) {
    if (shm_ptr == NULL || email == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < MAX_USERS; i++) {
        if (shm_ptr->users[i].is_active && 
            strcmp(shm_ptr->users[i].email, email) == 0) {
            return &shm_ptr->users[i];
        }
    }
    
    return NULL;
}

int update_user(SharedMemoryData* shm_ptr, int user_id, const char* name, const char* email, const char* password, int age) {
    if (shm_ptr == NULL || name == NULL || email == NULL) {
        printf("Error: Invalid parameters\n");
        return 0;
    }
    
    User* user = read_user(shm_ptr, user_id);
    if (user == NULL) {
        printf("Error: User not found\n");
        return 0;
    }
    
    User* existing_user = find_user_by_email(shm_ptr, email);
    if (existing_user != NULL && existing_user->user_id != user_id) {
        printf("Error: Email already exists for another user\n");
        return 0;
    }
    
    strncpy(user->name, name, MAX_NAME_LENGTH - 1);
    user->name[MAX_NAME_LENGTH - 1] = '\0';
    strncpy(user->email, email, MAX_EMAIL_LENGTH - 1);
    user->email[MAX_EMAIL_LENGTH - 1] = '\0';
    
    if (password != NULL && strlen(password) > 0) {
        strncpy(user->password, password, MAX_PASSWORD_LENGTH - 1);
        user->password[MAX_PASSWORD_LENGTH - 1] = '\0';
    }
    
    user->age = age;
    
    printf("User updated successfully\n");
    return 1;
}

int delete_user(SharedMemoryData* shm_ptr, int user_id) {
    if (shm_ptr == NULL || user_id <= 0) {
        printf("Error: Invalid parameters\n");
        return 0;
    }
    
    User* user = read_user(shm_ptr, user_id);
    if (user == NULL) {
        printf("Error: User not found\n");
        return 0;
    }
    
    user->is_active = 0;
    shm_ptr->control.user_count--;
    
    printf("User deleted successfully\n");
    return 1;
}

void display_all_users(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    printf("\n=== ALL USERS ===\n");
    printf("%-5s %-20s %-30s %-5s %-20s\n", "ID", "Name", "Email", "Age", "Created");
    printf("--------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        if (shm_ptr->users[i].is_active) {
            User* user = &shm_ptr->users[i];
            
            char created_time[20];
            struct tm* tm_info = localtime(&user->created_at);
            strftime(created_time, sizeof(created_time), "%Y-%m-%d %H:%M", tm_info);
            
            printf("%-5d %-20.20s %-30.30s %-5d %-20s\n", 
                   user->user_id, user->name, user->email, user->age, created_time);
            count++;
        }
    }
    
    if (count == 0) {
        printf("No users found.\n");
    } else {
        printf("\nTotal: %d users\n", count);
    }
}

void search_users(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    char keyword[100];
    printf("\n=== SEARCH USERS ===\n");
    printf("Enter search keyword (name or email): ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0;
    
    printf("\nSearch results for '%s':\n", keyword);
    printf("%-5s %-20s %-30s %-5s\n", "ID", "Name", "Email", "Age");
    printf("----------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < MAX_USERS; i++) {
        if (shm_ptr->users[i].is_active) {
            User* user = &shm_ptr->users[i];
            if (strstr(user->name, keyword) != NULL || 
                strstr(user->email, keyword) != NULL) {
                printf("%-5d %-20.20s %-30.30s %-5d\n", 
                       user->user_id, user->name, user->email, user->age);
                count++;
            }
        }
    }
    
    if (count == 0) {
        printf("No users found matching '%s'.\n", keyword);
    } else {
        printf("\nFound %d users\n", count);
    }
}

void register_user(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    char name[MAX_NAME_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int age;
    
    printf("\n=== USER REGISTRATION ===\n");
    
    printf("Enter name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    
    printf("Enter email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;
    
    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    
    printf("Enter age: ");
    scanf("%d", &age);
    getchar(); 
    
    if (strlen(name) == 0) {
        printf("Error: Name cannot be empty\n");
        return;
    }
    
    if (strlen(email) == 0 || strchr(email, '@') == NULL) {
        printf("Error: Invalid email format\n");
        return;
    }
    
    if (strlen(password) < 4) {
        printf("Error: Password must be at least 4 characters\n");
        return;
    }
    
    if (age <= 0 || age > 150) {
        printf("Error: Invalid age\n");
        return;
    }
    
    int user_id = create_user(shm_ptr, name, email, password, age);
    if (user_id > 0) {
        save_users_to_file(shm_ptr);
        printf("Registration successful! Your User ID is: %d\n", user_id);
        printf("Please login with your credentials.\n");
    }
}

void edit_user(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    int user_id;
    
    if (is_user_logged_in()) {
        user_id = get_current_user_id();
        printf("\n=== UPDATE YOUR PROFILE ===\n");
    } else {
        printf("\n=== EDIT USER ===\n");
        printf("Enter User ID to edit: ");
        scanf("%d", &user_id);
        getchar(); 
    }
    
    User* user = read_user(shm_ptr, user_id);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    printf("\nCurrent information:\n");
    printf("Name: %s\n", user->name);
    printf("Email: %s\n", user->email);
    printf("Age: %d\n", user->age);
    
    char name[MAX_NAME_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int age;
    
    printf("\nEnter new information (press Enter to keep current value):\n");
    
    printf("New name [%s]: ", user->name);
    fgets(name, sizeof(name), stdin);
    if (name[0] == '\n') {
        strcpy(name, user->name);
    } else {
        name[strcspn(name, "\n")] = 0;
    }
    
    printf("New email [%s]: ", user->email);
    fgets(email, sizeof(email), stdin);
    if (email[0] == '\n') {
        strcpy(email, user->email);
    } else {
        email[strcspn(email, "\n")] = 0;
    }
    
    printf("New password (press Enter to keep current): ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    
    printf("New age [%d]: ", user->age);
    char age_str[10];
    fgets(age_str, sizeof(age_str), stdin);
    if (age_str[0] == '\n') {
        age = user->age;
    } else {
        age = atoi(age_str);
    }
    
    if (update_user(shm_ptr, user_id, name, email, strlen(password) > 0 ? password : NULL, age)) {
        save_users_to_file(shm_ptr);
        printf("User information updated successfully!\n");
    }
}

void remove_user(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    int user_id;
    printf("\n=== DELETE USER ===\n");
    printf("Enter User ID to delete: ");
    scanf("%d", &user_id);
    getchar(); 
    
    User* user = read_user(shm_ptr, user_id);
    if (user == NULL) {
        printf("User not found!\n");
        return;
    }
    
    printf("\nUser to delete:\n");
    printf("Name: %s\n", user->name);
    printf("Email: %s\n", user->email);
    printf("Age: %d\n", user->age);
    
    char confirm;
    printf("\nAre you sure you want to delete this user? (y/n): ");
    scanf("%c", &confirm);
    getchar(); 
    
    if (confirm == 'y' || confirm == 'Y') {
        if (delete_user(shm_ptr, user_id)) {
            save_users_to_file(shm_ptr);
            printf("User deleted successfully!\n");
        }
    } else {
        printf("User deletion cancelled.\n");
    }
}