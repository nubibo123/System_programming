#ifndef MAIL_SYSTEM_H
#define MAIL_SYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>

// Constants
#define MAX_USERS 100
#define MAX_EMAILS 1000
#define MAX_NAME_LENGTH 50
#define MAX_EMAIL_LENGTH 100
#define MAX_PASSWORD_LENGTH 50
#define MAX_SUBJECT_LENGTH 200
#define MAX_CONTENT_LENGTH 2000
#define USER_DB_FILE "users.txt"
#define EMAIL_DB_FILE "emails.txt"

// Shared Memory Keys
#define SHM_KEY_USERS 1234
#define SHM_KEY_EMAILS 5678
#define SHM_KEY_CONTROL 9999

// User Structure
typedef struct {
    int user_id;
    char name[MAX_NAME_LENGTH];
    char email[MAX_EMAIL_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int age;
    int is_active;
    time_t created_at;
} User;

// Email Structure
typedef struct {
    int email_id;
    int sender_id;
    int receiver_id;
    char subject[MAX_SUBJECT_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    time_t sent_at;
    int is_read;
    int is_deleted;
} Email;

// Control Structure for Shared Memory
typedef struct {
    int user_count;
    int email_count;
    int next_user_id;
    int next_email_id;
} ControlData;

// Shared Memory Structure
typedef struct {
    ControlData control;
    User users[MAX_USERS];
    Email emails[MAX_EMAILS];
} SharedMemoryData;

// Shared Memory Functions
int create_shared_memory();
SharedMemoryData* attach_shared_memory();
void detach_shared_memory(SharedMemoryData* shm_ptr);
void destroy_shared_memory();
void init_shared_memory(SharedMemoryData* shm_ptr);

// Database Functions
void save_users_to_file(SharedMemoryData* shm_ptr);
void load_users_from_file(SharedMemoryData* shm_ptr);
void save_emails_to_file(SharedMemoryData* shm_ptr);
void load_emails_from_file(SharedMemoryData* shm_ptr);

// User CRUD Functions
int create_user(SharedMemoryData* shm_ptr, const char* name, const char* email, const char* password, int age);
User* read_user(SharedMemoryData* shm_ptr, int user_id);
User* find_user_by_email(SharedMemoryData* shm_ptr, const char* email);
int update_user(SharedMemoryData* shm_ptr, int user_id, const char* name, const char* email, const char* password, int age);
int delete_user(SharedMemoryData* shm_ptr, int user_id);
void display_all_users(SharedMemoryData* shm_ptr);

// Email CRUD Functions
int create_email(SharedMemoryData* shm_ptr, int sender_id, int receiver_id, 
                 const char* subject, const char* content);
Email* read_email(SharedMemoryData* shm_ptr, int email_id);
int update_email_status(SharedMemoryData* shm_ptr, int email_id, int is_read);
int delete_email(SharedMemoryData* shm_ptr, int email_id);
void display_user_emails(SharedMemoryData* shm_ptr, int user_id, int type); // type: 0=received, 1=sent
void display_all_emails(SharedMemoryData* shm_ptr);

// Mail System Functions
void compose_mail(SharedMemoryData* shm_ptr);
void view_sent_mails(SharedMemoryData* shm_ptr);
void view_received_mails(SharedMemoryData* shm_ptr);
void search_emails(SharedMemoryData* shm_ptr);

// Additional User Functions
void register_user(SharedMemoryData* shm_ptr);
void edit_user(SharedMemoryData* shm_ptr);
void remove_user(SharedMemoryData* shm_ptr);
void search_users(SharedMemoryData* shm_ptr);

// Additional Email Functions
void delete_mail(SharedMemoryData* shm_ptr);
void reply_mail(SharedMemoryData* shm_ptr);
int get_unread_email_count(SharedMemoryData* shm_ptr, int user_id);
int mark_all_emails_read(SharedMemoryData* shm_ptr, int user_id);
int delete_read_emails(SharedMemoryData* shm_ptr, int user_id);
void find_emails_by_sender(SharedMemoryData* shm_ptr, int sender_id);
void find_emails_by_receiver(SharedMemoryData* shm_ptr, int receiver_id);

// Authentication Functions
int is_user_logged_in();
int get_current_user_id();
void logout_user();
User* verify_user_credentials(SharedMemoryData* shm_ptr, const char* email, const char* password);
void show_login_menu();
int handle_authentication(SharedMemoryData* shm_ptr);

// Shared Memory Cleanup
void cleanup_shared_memory();
void display_shared_memory_info(SharedMemoryData* shm_ptr);

// Utility Functions
void clear_screen();
void pause_system();
int get_user_choice();
void display_menu();

#endif // MAIL_SYSTEM_H