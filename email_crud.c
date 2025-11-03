#include "mail_system.h"

// Tạo email mới (CREATE)
int create_email(SharedMemoryData* shm_ptr, int sender_id, int receiver_id, 
                 const char* subject, const char* content) {
    if (shm_ptr == NULL || subject == NULL || content == NULL) {
        printf("Error: Invalid parameters\n");
        return -1;
    }
    
    if (shm_ptr->control.email_count >= MAX_EMAILS) {
        printf("Error: Maximum number of emails reached\n");
        return -1;
    }
    
    // Kiểm tra sender và receiver có tồn tại không
    User* sender = read_user(shm_ptr, sender_id);
    User* receiver = read_user(shm_ptr, receiver_id);
    
    if (sender == NULL) {
        printf("Error: Sender not found\n");
        return -1;
    }
    
    if (receiver == NULL) {
        printf("Error: Receiver not found\n");
        return -1;
    }
    
    // Tìm vị trí trống trong array
    int index = -1;
    for (int i = 0; i < MAX_EMAILS; i++) {
        if (shm_ptr->emails[i].email_id == 0 || shm_ptr->emails[i].is_deleted) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        printf("Error: No available slot for new email\n");
        return -1;
    }
    
    // Tạo email mới
    Email* new_email = &shm_ptr->emails[index];
    new_email->email_id = shm_ptr->control.next_email_id++;
    new_email->sender_id = sender_id;
    new_email->receiver_id = receiver_id;
    strncpy(new_email->subject, subject, MAX_SUBJECT_LENGTH - 1);
    new_email->subject[MAX_SUBJECT_LENGTH - 1] = '\0';
    strncpy(new_email->content, content, MAX_CONTENT_LENGTH - 1);
    new_email->content[MAX_CONTENT_LENGTH - 1] = '\0';
    new_email->sent_at = time(NULL);
    new_email->is_read = 0;
    new_email->is_deleted = 0;
    
    // Cập nhật index nếu cần
    if (index >= shm_ptr->control.email_count) {
        shm_ptr->control.email_count = index + 1;
    }
    
    return new_email->email_id;
}

// Đọc email theo ID (READ)
Email* read_email(SharedMemoryData* shm_ptr, int email_id) {
    if (shm_ptr == NULL || email_id <= 0) {
        return NULL;
    }
    
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        if (shm_ptr->emails[i].email_id == email_id && 
            !shm_ptr->emails[i].is_deleted) {
            return &shm_ptr->emails[i];
        }
    }
    
    return NULL;
}

// Cập nhật trạng thái đọc của email (UPDATE)
int update_email_status(SharedMemoryData* shm_ptr, int email_id, int is_read) {
    if (shm_ptr == NULL || email_id <= 0) {
        printf("Error: Invalid parameters\n");
        return 0;
    }
    
    Email* email = read_email(shm_ptr, email_id);
    if (email == NULL) {
        printf("Error: Email not found\n");
        return 0;
    }
    
    email->is_read = is_read;
    return 1;
}

// Xóa email (DELETE - soft delete)
int delete_email(SharedMemoryData* shm_ptr, int email_id) {
    if (shm_ptr == NULL || email_id <= 0) {
        printf("Error: Invalid parameters\n");
        return 0;
    }
    
    Email* email = read_email(shm_ptr, email_id);
    if (email == NULL) {
        printf("Error: Email not found\n");
        return 0;
    }
    
    email->is_deleted = 1;
    return 1;
}

// Hiển thị emails của một user cụ thể (type: 0=received, 1=sent)
void display_user_emails(SharedMemoryData* shm_ptr, int user_id, int type) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    User* user = read_user(shm_ptr, user_id);
    if (user == NULL) {
        printf("Error: User not found\n");
        return;
    }
    
    const char* title = (type == 0) ? "RECEIVED EMAILS" : "SENT EMAILS";
    printf("\n=== %s for %s ===\n", title, user->name);
    printf("%-5s %-20s %-30s %-20s %-10s\n", "ID", "From/To", "Subject", "Date", "Status");
    printf("-------------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted) {
            int match = 0;
            User* other_user = NULL;
            
            if (type == 0 && email->receiver_id == user_id) {
                // Received emails
                match = 1;
                other_user = read_user(shm_ptr, email->sender_id);
            } else if (type == 1 && email->sender_id == user_id) {
                // Sent emails
                match = 1;
                other_user = read_user(shm_ptr, email->receiver_id);
            }
            
            if (match) {
                char date_time[20];
                struct tm* tm_info = localtime(&email->sent_at);
                strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M", tm_info);
                
                printf("%-5d %-20.20s %-30.30s %-20s %-10s\n", 
                       email->email_id,
                       other_user ? other_user->email : "Unknown",
                       email->subject,
                       date_time,
                       email->is_read ? "Read" : "Unread");
                count++;
            }
        }
    }
    
    if (count == 0) {
        printf("No %s emails found.\n", (type == 0) ? "received" : "sent");
    } else {
        printf("\nTotal: %d emails\n", count);
    }
}

// Hiển thị tất cả emails trong hệ thống
void display_all_emails(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    printf("\n=== ALL EMAILS IN SYSTEM ===\n");
    printf("%-5s %-20s %-20s %-30s %-20s %-10s\n", "ID", "From", "To", "Subject", "Sent", "Status");
    printf("------------------------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted) {
            User* sender = read_user(shm_ptr, email->sender_id);
            User* receiver = read_user(shm_ptr, email->receiver_id);
            
            char sent_time[20];
            struct tm* tm_info = localtime(&email->sent_at);
            strftime(sent_time, sizeof(sent_time), "%Y-%m-%d %H:%M", tm_info);
            
            printf("%-5d %-20.20s %-20.20s %-30.30s %-20s %-10s\n", 
                   email->email_id,
                   sender ? sender->email : "Unknown",
                   receiver ? receiver->email : "Unknown",
                   email->subject,
                   sent_time,
                   email->is_read ? "Read" : "Unread");
            count++;
        }
    }
    
    if (count == 0) {
        printf("No emails found in the system.\n");
    } else {
        printf("\nTotal: %d emails\n", count);
    }
}

// Lấy số lượng email chưa đọc của user
int get_unread_email_count(SharedMemoryData* shm_ptr, int user_id) {
    if (shm_ptr == NULL || user_id <= 0) {
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted && 
            email->receiver_id == user_id && 
            !email->is_read) {
            count++;
        }
    }
    
    return count;
}

// Đánh dấu tất cả email của user là đã đọc
int mark_all_emails_read(SharedMemoryData* shm_ptr, int user_id) {
    if (shm_ptr == NULL || user_id <= 0) {
        printf("Error: Invalid parameters\n");
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted && 
            email->receiver_id == user_id && 
            !email->is_read) {
            email->is_read = 1;
            count++;
        }
    }
    
    printf("Marked %d emails as read\n", count);
    return count;
}

// Xóa tất cả email đã đọc của user
int delete_read_emails(SharedMemoryData* shm_ptr, int user_id) {
    if (shm_ptr == NULL || user_id <= 0) {
        printf("Error: Invalid parameters\n");
        return 0;
    }
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted && 
            (email->sender_id == user_id || email->receiver_id == user_id) && 
            email->is_read) {
            email->is_deleted = 1;
            count++;
        }
    }
    
    printf("Deleted %d read emails\n", count);
    return count;
}

// Tìm kiếm email theo người gửi
void find_emails_by_sender(SharedMemoryData* shm_ptr, int sender_id) {
    if (shm_ptr == NULL || sender_id <= 0) {
        printf("Error: Invalid parameters\n");
        return;
    }
    
    User* sender = read_user(shm_ptr, sender_id);
    if (sender == NULL) {
        printf("Error: Sender not found\n");
        return;
    }
    
    printf("\n=== EMAILS FROM %s ===\n", sender->email);
    printf("%-5s %-20s %-30s %-20s %-10s\n", "ID", "To", "Subject", "Sent", "Status");
    printf("-------------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted && email->sender_id == sender_id) {
            User* receiver = read_user(shm_ptr, email->receiver_id);
            
            char sent_time[20];
            struct tm* tm_info = localtime(&email->sent_at);
            strftime(sent_time, sizeof(sent_time), "%Y-%m-%d %H:%M", tm_info);
            
            printf("%-5d %-20.20s %-30.30s %-20s %-10s\n", 
                   email->email_id,
                   receiver ? receiver->email : "Unknown",
                   email->subject,
                   sent_time,
                   email->is_read ? "Read" : "Unread");
            count++;
        }
    }
    
    if (count == 0) {
        printf("No emails found from this sender.\n");
    } else {
        printf("\nTotal: %d emails\n", count);
    }
}

// Tìm kiếm email theo người nhận
void find_emails_by_receiver(SharedMemoryData* shm_ptr, int receiver_id) {
    if (shm_ptr == NULL || receiver_id <= 0) {
        printf("Error: Invalid parameters\n");
        return;
    }
    
    User* receiver = read_user(shm_ptr, receiver_id);
    if (receiver == NULL) {
        printf("Error: Receiver not found\n");
        return;
    }
    
    printf("\n=== EMAILS TO %s ===\n", receiver->email);
    printf("%-5s %-20s %-30s %-20s %-10s\n", "ID", "From", "Subject", "Sent", "Status");
    printf("-------------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted && email->receiver_id == receiver_id) {
            User* sender = read_user(shm_ptr, email->sender_id);
            
            char sent_time[20];
            struct tm* tm_info = localtime(&email->sent_at);
            strftime(sent_time, sizeof(sent_time), "%Y-%m-%d %H:%M", tm_info);
            
            printf("%-5d %-20.20s %-30.30s %-20s %-10s\n", 
                   email->email_id,
                   sender ? sender->email : "Unknown",
                   email->subject,
                   sent_time,
                   email->is_read ? "Read" : "Unread");
            count++;
        }
    }
    
    if (count == 0) {
        printf("No emails found for this receiver.\n");
    } else {
        printf("\nTotal: %d emails\n", count);
    }
}