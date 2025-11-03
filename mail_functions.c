#include "mail_system.h"

void compose_mail(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    if (!is_user_logged_in()) {
        printf("Error: User not logged in\n");
        return;
    }
    
    char receiver_email[MAX_EMAIL_LENGTH];
    char subject[MAX_SUBJECT_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    
    printf("\n=== COMPOSE NEW EMAIL ===\n");
    
    User* sender = read_user(shm_ptr, get_current_user_id());
    if (sender == NULL) {
        printf("Error: Current user not found\n");
        return;
    }
    
    printf("From: %s <%s>\n", sender->name, sender->email);
    
    printf("Receiver email: ");
    fgets(receiver_email, sizeof(receiver_email), stdin);
    receiver_email[strcspn(receiver_email, "\n")] = 0;
    
    User* receiver = find_user_by_email(shm_ptr, receiver_email);
    if (receiver == NULL) {
        printf("Receiver email not found!\n");
        return;
    }
    
    printf("Subject: ");
    fgets(subject, sizeof(subject), stdin);
    subject[strcspn(subject, "\n")] = 0;
    
    printf("Content (press Enter twice to finish):\n");
    content[0] = '\0';
    char line[500];
    
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        if (strlen(line) == 1 && line[0] == '\n') break; // Empty line to finish
        
        if (strlen(content) + strlen(line) < MAX_CONTENT_LENGTH - 1) {
            strcat(content, line);
        } else {
            printf("Content too long! Truncating...\n");
            break;
        }
    }
    
    int email_id = create_email(shm_ptr, sender->user_id, receiver->user_id, subject, content);
    if (email_id > 0) {
        printf("Email sent successfully! Email ID: %d\n", email_id);
        save_emails_to_file(shm_ptr);
    } else {
        printf("Failed to send email!\n");
    }
}

void view_sent_mails(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    if (!is_user_logged_in()) {
        printf("Error: User not logged in\n");
        return;
    }
    
    printf("\n=== SENT EMAILS ===\n");
    
    User* user = read_user(shm_ptr, get_current_user_id());
    if (user == NULL) {
        printf("Error: Current user not found\n");
        return;
    }
    
    printf("\nEmails sent by %s:\n", user->name);
    printf("%-5s %-20s %-30s %-20s %-10s\n", "ID", "To", "Subject", "Sent", "Status");
    printf("-------------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted && email->sender_id == user->user_id) {
            User* receiver = read_user(shm_ptr, email->receiver_id);
            
            char sent_time[20];
            struct tm* tm_info = localtime(&email->sent_at);
            strftime(sent_time, sizeof(sent_time), "%Y-%m-%d %H:%M", tm_info);
            
            printf("%-5d %-20s %-30.30s %-20s %-10s\n", 
                   email->email_id,
                   receiver ? receiver->email : "Unknown",
                   email->subject,
                   sent_time,
                   email->is_read ? "Read" : "Unread");
            count++;
        }
    }
    
    if (count == 0) {
        printf("No sent emails found.\n");
    } else {
        printf("\nTotal: %d emails\n", count);
        
        // Option to view email detail
        char choice;
        printf("\nDo you want to view an email? (y/n): ");
        scanf("%c", &choice);
        getchar(); // Clear buffer
        
        if (choice == 'y' || choice == 'Y') {
            int email_id;
            printf("Enter email ID to view: ");
            scanf("%d", &email_id);
            getchar(); // Clear buffer
            
            Email* email = read_email(shm_ptr, email_id);
            if (email == NULL) {
                printf("Email not found!\n");
                return;
            }
            
            // Check if user is sender
            if (email->sender_id != get_current_user_id()) {
                printf("Error: You can only view emails you sent!\n");
                return;
            }
            
            User* sender = read_user(shm_ptr, email->sender_id);
            User* receiver = read_user(shm_ptr, email->receiver_id);
            
            printf("\n╔════════════════════════════════════════════════════════╗\n");
            printf("║                    EMAIL DETAILS                       ║\n");
            printf("╚════════════════════════════════════════════════════════╝\n");
            printf("\n📧 Email ID: %d\n", email->email_id);
            printf("👤 From: %s <%s>\n", sender ? sender->name : "Unknown", sender ? sender->email : "unknown@email.com");
            printf("👤 To: %s <%s>\n", receiver ? receiver->name : "Unknown", receiver ? receiver->email : "unknown@email.com");
            printf("📌 Subject: %s\n", email->subject);
            printf("📅 Sent: %s", ctime(&email->sent_at));
            printf("📊 Status: %s\n", email->is_read ? "Read by receiver" : "Unread");
            printf("\n" "────────────────────────────────────────────────────────\n");
            printf("📄 Content:\n\n%s\n", email->content);
            printf("────────────────────────────────────────────────────────\n\n");
        }
    }
}

void view_received_mails(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    if (!is_user_logged_in()) {
        printf("Error: User not logged in\n");
        return;
    }
    
    printf("\n=== RECEIVED EMAILS ===\n");
    
    User* user = read_user(shm_ptr, get_current_user_id());
    if (user == NULL) {
        printf("Error: Current user not found\n");
        return;
    }
    
    printf("\nEmails received by %s:\n", user->name);
    printf("%-5s %-20s %-30s %-20s %-10s\n", "ID", "From", "Subject", "Received", "Status");
    printf("-------------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted && email->receiver_id == user->user_id) {
            User* sender = read_user(shm_ptr, email->sender_id);
            
            char received_time[20];
            struct tm* tm_info = localtime(&email->sent_at);
            strftime(received_time, sizeof(received_time), "%Y-%m-%d %H:%M", tm_info);
            
            printf("%-5d %-20s %-30.30s %-20s %-10s\n", 
                   email->email_id,
                   sender ? sender->email : "Unknown",
                   email->subject,
                   received_time,
                   email->is_read ? "Read" : "Unread");
            count++;
        }
    }
    
    if (count == 0) {
        printf("No received emails found.\n");
    } else {
        printf("\nTotal: %d emails\n", count);
        
        // Option to read email detail
        char choice;
        printf("\nDo you want to read an email? (y/n): ");
        scanf("%c", &choice);
        getchar(); // Clear buffer
        
        if (choice == 'y' || choice == 'Y') {
            int email_id;
            printf("Enter email ID to read: ");
            scanf("%d", &email_id);
            getchar(); // Clear buffer
            
            Email* email = read_email(shm_ptr, email_id);
            if (email == NULL) {
                printf("Email not found!\n");
                return;
            }
            
            // Check if user is receiver
            if (email->receiver_id != get_current_user_id()) {
                printf("Error: You can only read emails sent to you!\n");
                return;
            }
            
            User* sender = read_user(shm_ptr, email->sender_id);
            User* receiver = read_user(shm_ptr, email->receiver_id);
            
            printf("\n╔════════════════════════════════════════════════════════╗\n");
            printf("║                    EMAIL DETAILS                       ║\n");
            printf("╚════════════════════════════════════════════════════════╝\n");
            printf("\nEmail ID: %d\n", email->email_id);
            printf("From: %s <%s>\n", sender ? sender->name : "Unknown", sender ? sender->email : "unknown@email.com");
            printf("To: %s <%s>\n", receiver ? receiver->name : "Unknown", receiver ? receiver->email : "unknown@email.com");
            printf("Subject: %s\n", email->subject);
            printf("Sent: %s", ctime(&email->sent_at));
            printf("Status: %s\n", email->is_read ? "Read" : "Unread");
            printf("\n" "────────────────────────────────────────────────────────\n");
            printf("📄 Content:\n\n%s\n", email->content);
            printf("────────────────────────────────────────────────────────\n\n");
            
            if (!email->is_read) {
                update_email_status(shm_ptr, email_id, 1);
                printf("✓ Email marked as read.\n");
                save_emails_to_file(shm_ptr);
            }
        }
    }
}

void search_emails(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    char keyword[200];
    printf("\n=== SEARCH EMAILS ===\n");
    printf("Enter search keyword (subject or content): ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0;
    
    printf("\nSearch results for '%s':\n", keyword);
    printf("%-5s %-20s %-20s %-30s %-10s\n", "ID", "From", "To", "Subject", "Status");
    printf("-------------------------------------------------------------------------------------\n");
    
    int count = 0;
    for (int i = 0; i < shm_ptr->control.email_count; i++) {
        Email* email = &shm_ptr->emails[i];
        if (!email->is_deleted) {
            if (strstr(email->subject, keyword) != NULL || 
                strstr(email->content, keyword) != NULL) {
                
                User* sender = read_user(shm_ptr, email->sender_id);
                User* receiver = read_user(shm_ptr, email->receiver_id);
                
                printf("%-5d %-20s %-20s %-30.30s %-10s\n", 
                       email->email_id,
                       sender ? sender->email : "Unknown",
                       receiver ? receiver->email : "Unknown",
                       email->subject,
                       email->is_read ? "Read" : "Unread");
                count++;
            }
        }
    }
    
    if (count == 0) {
        printf("No emails found matching '%s'.\n", keyword);
    } else {
        printf("\nFound %d emails\n", count);
    }
}

void delete_mail(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    int email_id;
    printf("\n=== DELETE EMAIL ===\n");
    printf("Enter email ID to delete: ");
    scanf("%d", &email_id);
    getchar(); 
    
    Email* email = read_email(shm_ptr, email_id);
    if (email == NULL) {
        printf("Email not found!\n");
        return;
    }
    
    User* sender = read_user(shm_ptr, email->sender_id);
    User* receiver = read_user(shm_ptr, email->receiver_id);
    
    printf("\nEmail to delete:\n");
    printf("From: %s <%s>\n", sender ? sender->name : "Unknown", sender ? sender->email : "unknown");
    printf("To: %s <%s>\n", receiver ? receiver->name : "Unknown", receiver ? receiver->email : "unknown");
    printf("Subject: %s\n", email->subject);
    
    char confirm;
    printf("\nAre you sure you want to delete this email? (y/n): ");
    scanf("%c", &confirm);
    getchar(); // Clear buffer
    
    if (confirm == 'y' || confirm == 'Y') {
        if (delete_email(shm_ptr, email_id)) {
            printf("Email deleted successfully!\n");
            save_emails_to_file(shm_ptr);
        } else {
            printf("Failed to delete email!\n");
        }
    } else {
        printf("Email deletion cancelled.\n");
    }
}

void reply_mail(SharedMemoryData* shm_ptr) {
    if (shm_ptr == NULL) {
        printf("Error: Shared memory not available\n");
        return;
    }
    
    int email_id;
    printf("\n=== REPLY TO EMAIL ===\n");
    printf("Enter email ID to reply: ");
    scanf("%d", &email_id);
    getchar(); 
    
    Email* original_email = read_email(shm_ptr, email_id);
    if (original_email == NULL) {
        printf("Email not found!\n");
        return;
    }
    
    User* original_sender = read_user(shm_ptr, original_email->sender_id);
    User* receiver = read_user(shm_ptr, original_email->receiver_id);
    
    printf("\nReplying to:\n");
    printf("From: %s <%s>\n", original_sender ? original_sender->name : "Unknown", 
           original_sender ? original_sender->email : "unknown");
    printf("Subject: %s\n", original_email->subject);
    
    char content[MAX_CONTENT_LENGTH];
    printf("\nYour reply content (press Enter twice to finish):\n");
    content[0] = '\0';
    char line[500];
    
    while (1) {
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        if (strlen(line) == 1 && line[0] == '\n') break;
        
        if (strlen(content) + strlen(line) < MAX_CONTENT_LENGTH - 1) {
            strcat(content, line);
        } else {
            printf("Content too long! Truncating...\n");
            break;
        }
    }
    
    char reply_subject[MAX_SUBJECT_LENGTH];
    if (strncmp(original_email->subject, "Re: ", 4) == 0) {
        strcpy(reply_subject, original_email->subject);
    } else {
        snprintf(reply_subject, sizeof(reply_subject), "Re: %s", original_email->subject);
    }
    
    int reply_id = create_email(shm_ptr, receiver->user_id, original_email->sender_id, 
                               reply_subject, content);
    if (reply_id > 0) {
        printf("Reply sent successfully! Email ID: %d\n", reply_id);
        save_emails_to_file(shm_ptr);
    } else {
        printf("Failed to send reply!\n");
    }
}