#define _GNU_SOURCE
#include "mail_system.h"

// Lưu danh sách users vào file
void save_users_to_file(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory pointer is NULL\n");
        return;
    }
    
    FILE* file = fopen(USER_DB_FILE, "w");
    if (file == NULL) {
        perror("Error opening users database file for writing");
        return;
    }
    
    // Ghi header với control data
    fprintf(file, "# Users Database - Text Format\n");
    fprintf(file, "# USER_COUNT: %d\n", shm_ptr->control.user_count);
    fprintf(file, "# NEXT_USER_ID: %d\n", shm_ptr->control.next_user_id);
    fprintf(file, "# Format: ID|Name|Email|Password|Age|IsActive|CreatedAt\n");
    fprintf(file, "# ==========================================\n");
    
    // Ghi danh sách users
    for (int i = 0; i < shm_ptr->control.user_count; i++) {
        if (shm_ptr->users[i].is_active) {
            fprintf(file, "%d|%s|%s|%s|%d|%d|%ld\n",
                    shm_ptr->users[i].user_id,
                    shm_ptr->users[i].name,
                    shm_ptr->users[i].email,
                    shm_ptr->users[i].password,
                    shm_ptr->users[i].age,
                    shm_ptr->users[i].is_active,
                    shm_ptr->users[i].created_at);
        }
    }
    
    fclose(file);
    printf("Users data saved to %s successfully\n", USER_DB_FILE);
}

// Đọc danh sách users từ file
void load_users_from_file(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory pointer is NULL\n");
        return;
    }
    
    FILE* file = fopen(USER_DB_FILE, "r");
    if (file == NULL) {
        printf("No existing users database file found. Starting with empty database.\n");
        return;
    }
    
    char line[512];
    int saved_user_count = 0, saved_next_user_id = 1;
    
    // Đọc header và control data
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "# USER_COUNT:", 13) == 0) {
            sscanf(line + 13, "%d", &saved_user_count);
        } else if (strncmp(line, "# NEXT_USER_ID:", 15) == 0) {
            sscanf(line + 15, "%d", &saved_next_user_id);
        } else if (line[0] != '#' && strlen(line) > 1) {
            // Đây là data line, break để đọc users
            fseek(file, -strlen(line), SEEK_CUR);
            break;
        }
    }
    
    // Đọc danh sách users
    shm_ptr->control.user_count = 0;
    shm_ptr->control.next_user_id = saved_next_user_id;
    
    while (fgets(line, sizeof(line), file) && 
           shm_ptr->control.user_count < MAX_USERS) {
        if (line[0] == '#' || strlen(line) <= 1) continue;
        
        User* user = &shm_ptr->users[shm_ptr->control.user_count];
        
        // Parse: ID|Name|Email|Password|Age|IsActive|CreatedAt
        char* token = strtok(line, "|");
        if (token) user->user_id = atoi(token);
        
        token = strtok(NULL, "|");
        if (token) strncpy(user->name, token, MAX_NAME_LENGTH - 1);
        
        token = strtok(NULL, "|");
        if (token) strncpy(user->email, token, MAX_EMAIL_LENGTH - 1);
        
        token = strtok(NULL, "|");
        if (token) strncpy(user->password, token, MAX_PASSWORD_LENGTH - 1);
        
        token = strtok(NULL, "|");
        if (token) user->age = atoi(token);
        
        token = strtok(NULL, "|");
        if (token) user->is_active = atoi(token);
        
        token = strtok(NULL, "|");
        if (token) user->created_at = atol(token);
        
        if (user->is_active) {
            shm_ptr->control.user_count++;
        }
    }
    
    fclose(file);
    printf("Loaded %d users from %s\n", shm_ptr->control.user_count, USER_DB_FILE);
}

// Lưu danh sách emails vào file
void save_emails_to_file(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory pointer is NULL\n");
        return;
    }
    
    FILE* file = fopen(EMAIL_DB_FILE, "w");
    if (file == NULL) {
        perror("Error opening emails database file for writing");
        return;
    }
    
    // Ghi header với control data
    fprintf(file, "# Emails Database - Text Format\n");
    fprintf(file, "# EMAIL_COUNT: %d\n", shm_ptr->control.email_count);
    fprintf(file, "# NEXT_EMAIL_ID: %d\n", shm_ptr->control.next_email_id);
    fprintf(file, "# Format: ID|SenderID|ReceiverID|Subject|Content|SentAt|IsRead|IsDeleted\n");
    fprintf(file, "# ======================================================================\n");
    
    // Ghi danh sách emails
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        if (!shm_ptr->emails[i].is_deleted) {
            // Escape các ký tự đặc biệt trong subject và content
            char escaped_subject[MAX_SUBJECT_LENGTH * 2];
            char escaped_content[MAX_CONTENT_LENGTH * 2];
            
            // Simple escape: thay | bằng &#124; và newline bằng \\n
            int j = 0;
            for (int k = 0; shm_ptr->emails[i].subject[k] && j < sizeof(escaped_subject) - 10; k++) {
                if (shm_ptr->emails[i].subject[k] == '|') {
                    strcpy(escaped_subject + j, "&#124;");
                    j += 6;
                } else if (shm_ptr->emails[i].subject[k] == '\n') {
                    strcpy(escaped_subject + j, "\\n");
                    j += 2;
                } else {
                    escaped_subject[j++] = shm_ptr->emails[i].subject[k];
                }
            }
            escaped_subject[j] = '\0';
            
            j = 0;
            for (int k = 0; shm_ptr->emails[i].content[k] && j < sizeof(escaped_content) - 10; k++) {
                if (shm_ptr->emails[i].content[k] == '|') {
                    strcpy(escaped_content + j, "&#124;");
                    j += 6;
                } else if (shm_ptr->emails[i].content[k] == '\n') {
                    strcpy(escaped_content + j, "\\n");
                    j += 2;
                } else {
                    escaped_content[j++] = shm_ptr->emails[i].content[k];
                }
            }
            escaped_content[j] = '\0';
            
            fprintf(file, "%d|%d|%d|%s|%s|%ld|%d|%d\n",
                    shm_ptr->emails[i].email_id,
                    shm_ptr->emails[i].sender_id,
                    shm_ptr->emails[i].receiver_id,
                    escaped_subject,
                    escaped_content,
                    shm_ptr->emails[i].sent_at,
                    shm_ptr->emails[i].is_read,
                    shm_ptr->emails[i].is_deleted);
        }
    }
    
    fclose(file);
    printf("Emails data saved to %s successfully\n", EMAIL_DB_FILE);
}

// Đọc danh sách emails từ file
void load_emails_from_file(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory pointer is NULL\n");
        return;
    }
    
    FILE* file = fopen(EMAIL_DB_FILE, "r");
    if (file == NULL) {
        printf("No existing emails database file found. Starting with empty database.\n");
        return;
    }
    
    char line[4096];  // Larger buffer for content
    int saved_email_count = 0, saved_next_email_id = 1;
    
    // Đọc header và control data
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "# EMAIL_COUNT:", 14) == 0) {
            sscanf(line + 14, "%d", &saved_email_count);
        } else if (strncmp(line, "# NEXT_EMAIL_ID:", 16) == 0) {
            sscanf(line + 16, "%d", &saved_next_email_id);
        } else if (line[0] != '#' && strlen(line) > 1) {
            // Đây là data line, break để đọc emails
            fseek(file, -strlen(line), SEEK_CUR);
            break;
        }
    }
    
    // Đọc danh sách emails
    shm_ptr->control.email_count = 0;
    shm_ptr->control.next_email_id = saved_next_email_id;
    
    while (fgets(line, sizeof(line), file) && 
           shm_ptr->control.email_count < MAX_EMAILS) {
        if (line[0] == '#' || strlen(line) <= 1) continue;
        
        Email* email = &shm_ptr->emails[shm_ptr->control.email_count];
        
        // Parse: ID|SenderID|ReceiverID|Subject|Content|SentAt|IsRead|IsDeleted
        char* saveptr;
        char* token = strtok_r(line, "|", &saveptr);
        if (token) email->email_id = atoi(token);
        
        token = strtok_r(NULL, "|", &saveptr);
        if (token) email->sender_id = atoi(token);
        
        token = strtok_r(NULL, "|", &saveptr);
        if (token) email->receiver_id = atoi(token);
        
        token = strtok_r(NULL, "|", &saveptr);
        if (token) {
            // Unescape subject
            char* src = token;
            char* dst = email->subject;
            while (*src && dst - email->subject < MAX_SUBJECT_LENGTH - 1) {
                if (strncmp(src, "&#124;", 6) == 0) {
                    *dst++ = '|';
                    src += 6;
                } else if (strncmp(src, "\\n", 2) == 0) {
                    *dst++ = '\n';
                    src += 2;
                } else {
                    *dst++ = *src++;
                }
            }
            *dst = '\0';
        }
        
        token = strtok_r(NULL, "|", &saveptr);
        if (token) {
            // Unescape content
            char* src = token;
            char* dst = email->content;
            while (*src && dst - email->content < MAX_CONTENT_LENGTH - 1) {
                if (strncmp(src, "&#124;", 6) == 0) {
                    *dst++ = '|';
                    src += 6;
                } else if (strncmp(src, "\\n", 2) == 0) {
                    *dst++ = '\n';
                    src += 2;
                } else {
                    *dst++ = *src++;
                }
            }
            *dst = '\0';
        }
        
        token = strtok_r(NULL, "|", &saveptr);
        if (token) email->sent_at = atol(token);
        
        token = strtok_r(NULL, "|", &saveptr);
        if (token) email->is_read = atoi(token);
        
        token = strtok_r(NULL, "|", &saveptr);
        if (token) email->is_deleted = atoi(token);
        
        if (!email->is_deleted) {
            shm_ptr->control.email_count++;
        }
    }
    
    fclose(file);
    printf("Loaded %d emails from %s\n", shm_ptr->control.email_count, EMAIL_DB_FILE);
}

// Backup toàn bộ database
void backup_database(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory pointer is NULL\n");
        return;
    }
    
    char backup_users[100], backup_emails[100];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    // Tạo tên file backup với timestamp
    strftime(backup_users, sizeof(backup_users), "users_backup_%Y%m%d_%H%M%S.txt", tm_info);
    strftime(backup_emails, sizeof(backup_emails), "emails_backup_%Y%m%d_%H%M%S.txt", tm_info);
    
    // Backup users
    FILE* users_backup = fopen(backup_users, "w");
    if (users_backup != NULL) {
        fprintf(users_backup, "# Users Backup - Text Format\n");
        fprintf(users_backup, "# USER_COUNT: %d\n", shm_ptr->control.user_count);
        fprintf(users_backup, "# NEXT_USER_ID: %d\n", shm_ptr->control.next_user_id);
        fprintf(users_backup, "# Format: ID|Name|Email|Age|IsActive|CreatedAt\n");
        fprintf(users_backup, "# ==========================================\n");
        
        for (int i = 0; i < shm_ptr->control.user_count; i++) {
            if (shm_ptr->users[i].is_active) {
                fprintf(users_backup, "%d|%s|%s|%d|%d|%ld\n",
                        shm_ptr->users[i].user_id,
                        shm_ptr->users[i].name,
                        shm_ptr->users[i].email,
                        shm_ptr->users[i].age,
                        shm_ptr->users[i].is_active,
                        shm_ptr->users[i].created_at);
            }
        }
        fclose(users_backup);
        printf("Users backup created: %s\n", backup_users);
    }
    
    // Backup emails
    FILE* emails_backup = fopen(backup_emails, "w");
    if (emails_backup != NULL) {
        fprintf(emails_backup, "# Emails Backup - Text Format\n");
        fprintf(emails_backup, "# EMAIL_COUNT: %d\n", shm_ptr->control.email_count);
        fprintf(emails_backup, "# NEXT_EMAIL_ID: %d\n", shm_ptr->control.next_email_id);
        fprintf(emails_backup, "# Format: ID|SenderID|ReceiverID|Subject|Content|SentAt|IsRead|IsDeleted\n");
        fprintf(emails_backup, "# ======================================================================\n");
        
        for (int i = 0; i < shm_ptr->control.email_count; i++) {
            if (!shm_ptr->emails[i].is_deleted) {
                // Simple backup without escaping for readability
                fprintf(emails_backup, "%d|%d|%d|%s|%s|%ld|%d|%d\n",
                        shm_ptr->emails[i].email_id,
                        shm_ptr->emails[i].sender_id,
                        shm_ptr->emails[i].receiver_id,
                        shm_ptr->emails[i].subject,
                        shm_ptr->emails[i].content,
                        shm_ptr->emails[i].sent_at,
                        shm_ptr->emails[i].is_read,
                        shm_ptr->emails[i].is_deleted);
            }
        }
        fclose(emails_backup);
        printf("Emails backup created: %s\n", backup_emails);
    }
}

// Xóa database files
void clear_database_files() {
    if (remove(USER_DB_FILE) == 0) {
        printf("Users database file deleted successfully\n");
    } else {
        printf("Users database file not found or cannot be deleted\n");
    }
    
    if (remove(EMAIL_DB_FILE) == 0) {
        printf("Emails database file deleted successfully\n");
    } else {
        printf("Emails database file not found or cannot be deleted\n");
    }
}

// Kiểm tra tính toàn vẹn của database
int validate_database(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory pointer is NULL\n");
        return 0;
    }
    
    int valid = 1;
    
    // Kiểm tra users
    printf("Validating users database...\n");
    for (int i = 0; i < shm_ptr->control.user_count; i++) {
        if (shm_ptr->users[i].is_active) {
            if (shm_ptr->users[i].user_id <= 0 || 
                strlen(shm_ptr->users[i].name) == 0 ||
                strlen(shm_ptr->users[i].email) == 0) {
                printf("Invalid user found at index %d\n", i);
                valid = 0;
            }
        }
    }
    
    // Kiểm tra emails
    printf("Validating emails database...\n");
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        if (!shm_ptr->emails[i].is_deleted) {
            if (shm_ptr->emails[i].email_id <= 0 ||
                shm_ptr->emails[i].sender_id <= 0 ||
                shm_ptr->emails[i].receiver_id <= 0) {
                printf("Invalid email found at index %d\n", i);
                valid = 0;
            }
        }
    }
    
    if (valid) {
        printf("Database validation passed\n");
    } else {
        printf("Database validation failed\n");
    }
    
    return valid;
}