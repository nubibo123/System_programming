#include "mail_system.h"

static int shm_id = -1;

int create_shared_memory() {
    key_t key = ftok(".", SHM_KEY_USERS);
    if (key == -1) {
        perror("ftok failed");
        return -1;
    }
    
    shm_id = shmget(key, sizeof(SharedMemoryData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        return -1;
    }
    
    printf("Shared memory created successfully with ID: %d\n", shm_id);
    return shm_id;
}

SharedMemoryData* attach_shared_memory() {
    key_t key = ftok(".", SHM_KEY_USERS);
    if (key == -1) {
        perror("ftok failed");
        return NULL;
    }
    
    shm_id = shmget(key, sizeof(SharedMemoryData), 0666);
    if (shm_id == -1) {
        shm_id = create_shared_memory();
        if (shm_id == -1) {
            return NULL;
        }
    }
    
    SharedMemoryData* shm_ptr = (SharedMemoryData*) shmat(shm_id, NULL, 0);
    if (shm_ptr == (SharedMemoryData*) -1) {
        perror("shmat failed");
        return NULL;
    }
    
    return shm_ptr;
}

void detach_shared_memory(SharedMemoryData* shm_ptr) {
    if (shm_ptr != NULL) {
        if (shmdt(shm_ptr) == -1) {
            perror("shmdt failed");
        } else {
            printf("Successfully detached from shared memory\n");
        }
    }
}

void destroy_shared_memory() {
    if (shm_id != -1) {
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("shmctl IPC_RMID failed");
        } else {
            printf("Shared memory destroyed successfully\n");
            shm_id = -1;
        }
    }
}

void init_shared_memory(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory pointer is NULL\n");
        return;
    }
    
    if (shm_ptr->control.next_user_id == 0) {
        shm_ptr->control.user_count = 0;
        shm_ptr->control.email_count = 0;
        shm_ptr->control.next_user_id = 1;
        shm_ptr->control.next_email_id = 1;
        
        memset(shm_ptr->users, 0, sizeof(shm_ptr->users));
        memset(shm_ptr->emails, 0, sizeof(shm_ptr->emails));
        
        printf("Shared memory initialized successfully\n");
        
        load_users_from_file(shm_ptr);
        load_emails_from_file(shm_ptr);
    }
}

void display_shared_memory_info(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Shared memory not available\n");
        return;
    }
    
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║         SHARED MEMORY - REAL-TIME DATA VIEW           ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    // Control Information
    printf("\n📊 CONTROL INFORMATION:\n");
    printf("├─ Shared Memory ID: %d\n", shm_id);
    printf("├─ Memory Size: %.2f MB (%lu bytes)\n", 
           sizeof(SharedMemoryData) / (1024.0 * 1024.0),
           sizeof(SharedMemoryData));
    printf("├─ Total Users: %d / %d\n", shm_ptr->control.user_count, MAX_USERS);
    printf("├─ Total Emails: %d / %d\n", shm_ptr->control.email_count, MAX_EMAILS);
    printf("├─ Next User ID: %d\n", shm_ptr->control.next_user_id);
    printf("└─ Next Email ID: %d\n", shm_ptr->control.next_email_id);
    
    // Users in Memory
    printf("\n👥 USERS IN SHARED MEMORY:\n");
    if (shm_ptr->control.user_count == 0) {
        printf("   (No users in memory)\n");
    } else {
        printf("┌─────┬──────────────────────┬────────────────────────────┬─────┬────────┐\n");
        printf("│ ID  │ Name                 │ Email                      │ Age │ Status │\n");
        printf("├─────┼──────────────────────┼────────────────────────────┼─────┼────────┤\n");
        for (int i = 0; i < MAX_USERS && shm_ptr->control.user_count > 0; i++) {
            if (shm_ptr->users[i].is_active) {
                printf("│ %-3d │ %-20.20s │ %-26.26s │ %-3d │ %-6s │\n",
                       shm_ptr->users[i].user_id,
                       shm_ptr->users[i].name,
                       shm_ptr->users[i].email,
                       shm_ptr->users[i].age,
                       shm_ptr->users[i].is_active ? "Active" : "Inact");
            }
        }
        printf("└─────┴──────────────────────┴────────────────────────────┴─────┴────────┘\n");
    }
    
    // Emails in Memory
    printf("\n📧 EMAILS IN SHARED MEMORY:\n");
    if (shm_ptr->control.email_count == 0) {
        printf("   (No emails in memory)\n");
    } else {
        printf("┌─────┬──────┬──────┬─────────────────────────────┬────────┬─────────┐\n");
        printf("│ ID  │ From │ To   │ Subject                     │ Status │ Deleted │\n");
        printf("├─────┼──────┼──────┼─────────────────────────────┼────────┼─────────┤\n");
        int count = 0;
        for (int i = 0; i < MAX_EMAILS && count < shm_ptr->control.email_count; i++) {
            if (shm_ptr->emails[i].email_id > 0) {
                printf("│ %-3d │ %-4d │ %-4d │ %-27.27s │ %-6s │ %-7s │\n",
                       shm_ptr->emails[i].email_id,
                       shm_ptr->emails[i].sender_id,
                       shm_ptr->emails[i].receiver_id,
                       shm_ptr->emails[i].subject,
                       shm_ptr->emails[i].is_read ? "Read" : "Unread",
                       shm_ptr->emails[i].is_deleted ? "Yes" : "No");
                count++;
            }
        }
        printf("└─────┴──────┴──────┴─────────────────────────────┴────────┴─────────┘\n");
    }
    
    // Memory Usage
    printf("\n💾 MEMORY USAGE:\n");
    size_t used_user_memory = shm_ptr->control.user_count * sizeof(User);
    size_t used_email_memory = shm_ptr->control.email_count * sizeof(Email);
    size_t total_used = sizeof(ControlData) + used_user_memory + used_email_memory;
    size_t total_allocated = sizeof(SharedMemoryData);
    
    printf("├─ Control Data: %lu bytes\n", sizeof(ControlData));
    printf("├─ Users: %lu bytes (%d active)\n", used_user_memory, shm_ptr->control.user_count);
    printf("├─ Emails: %lu bytes (%d active)\n", used_email_memory, shm_ptr->control.email_count);
    printf("├─ Total Used: %.2f KB / %.2f MB\n", 
           total_used / 1024.0, 
           total_allocated / (1024.0 * 1024.0));
    printf("└─ Usage: %.1f%%\n", (total_used * 100.0) / total_allocated);
    
    printf("\n════════════════════════════════════════════════════════\n");
}

int check_shared_memory_status() {
    key_t key = ftok(".", SHM_KEY_USERS);
    if (key == -1) {
        return 0; 
    }
    
    int temp_shm_id = shmget(key, sizeof(SharedMemoryData), 0666);
    return (temp_shm_id != -1) ? 1 : 0;
}

void cleanup_shared_memory() {
    if (shm_id != -1) {
        printf("Cleaning up shared memory...\n");
        
        SharedMemoryData* shm_ptr = attach_shared_memory();
        if (shm_ptr != NULL) {
            save_users_to_file(shm_ptr);
            save_emails_to_file(shm_ptr);
            detach_shared_memory(shm_ptr);
        }
        
        // Không destroy shared memory ở đây để các process khác có thể sử dụng
        // destroy_shared_memory();
    }
}