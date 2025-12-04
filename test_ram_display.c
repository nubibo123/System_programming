#include <gtk/gtk.h>
#include "mail_system.h"

// External variable from shared_memory.c
extern int shm_id;

typedef struct {
    GtkWidget *window;
    GtkWidget *shm_id_label;
    GtkWidget *shm_size_label;
    GtkWidget *users_label;
    GtkWidget *emails_label;
    GtkWidget *next_user_id_label;
    GtkWidget *next_email_id_label;
    GtkWidget *memory_used_label;
    GtkWidget *memory_total_label;
    GtkWidget *usage_label;
    GtkWidget *progress_bar;
    GtkWidget *status_label;
    GtkWidget *text_view;
    guint timer_id;
    SharedMemoryData *shm_ptr;
} AppWidgets;

// Update the GUI with shared memory information
gboolean update_ram_display(gpointer data) {
    AppWidgets *widgets = (AppWidgets *)data;
    char text[512];
    
    if (widgets->shm_ptr == NULL) {
        gtk_label_set_markup(GTK_LABEL(widgets->status_label), 
                            "<span foreground='red' weight='bold'>⚠ Shared Memory Not Available</span>");
        return TRUE;
    }
    
    // Update status
    gtk_label_set_markup(GTK_LABEL(widgets->status_label), 
                        "<span foreground='green' weight='bold'>✓ Shared Memory Active</span>");
    
    // Update shared memory ID
    sprintf(text, "<b>Shared Memory ID:</b> %d", shm_id);
    gtk_label_set_markup(GTK_LABEL(widgets->shm_id_label), text);
    
    // Update shared memory size
    size_t shm_size = sizeof(SharedMemoryData);
    sprintf(text, "<b>Memory Size:</b> %.2f MB (%lu bytes)", 
            shm_size / (1024.0 * 1024.0), shm_size);
    gtk_label_set_markup(GTK_LABEL(widgets->shm_size_label), text);
    
    // Update users count
    sprintf(text, "<b>Total Users:</b> %d / %d (%.1f%%)",
            widgets->shm_ptr->control.user_count,
            MAX_USERS,
            (widgets->shm_ptr->control.user_count * 100.0) / MAX_USERS);
    gtk_label_set_markup(GTK_LABEL(widgets->users_label), text);
    
    // Update emails count
    sprintf(text, "<b>Total Emails:</b> %d / %d (%.1f%%)",
            widgets->shm_ptr->control.email_count,
            MAX_EMAILS,
            (widgets->shm_ptr->control.email_count * 100.0) / MAX_EMAILS);
    gtk_label_set_markup(GTK_LABEL(widgets->emails_label), text);
    
    // Update next IDs
    sprintf(text, "<b>Next User ID:</b> %d", widgets->shm_ptr->control.next_user_id);
    gtk_label_set_markup(GTK_LABEL(widgets->next_user_id_label), text);
    
    sprintf(text, "<b>Next Email ID:</b> %d", widgets->shm_ptr->control.next_email_id);
    gtk_label_set_markup(GTK_LABEL(widgets->next_email_id_label), text);
    
    // Calculate memory usage
    size_t used_user_memory = widgets->shm_ptr->control.user_count * sizeof(User);
    size_t used_email_memory = widgets->shm_ptr->control.email_count * sizeof(Email);
    size_t total_used = sizeof(ControlData) + used_user_memory + used_email_memory;
    size_t total_allocated = sizeof(SharedMemoryData);
    double usage_percent = (total_used * 100.0) / total_allocated;
    
    sprintf(text, "<b>Memory Used:</b> %.2f KB (Control: %lu B, Users: %lu B, Emails: %lu B)",
            total_used / 1024.0,
            sizeof(ControlData),
            used_user_memory,
            used_email_memory);
    gtk_label_set_markup(GTK_LABEL(widgets->memory_used_label), text);
    
    sprintf(text, "<b>Memory Total:</b> %.2f MB", total_allocated / (1024.0 * 1024.0));
    gtk_label_set_markup(GTK_LABEL(widgets->memory_total_label), text);
    
    sprintf(text, "<b>Usage:</b> %.2f%%", usage_percent);
    gtk_label_set_markup(GTK_LABEL(widgets->usage_label), text);
    
    // Update progress bar
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->progress_bar), 
                                   usage_percent / 100.0);
    sprintf(text, "%.2f%%", usage_percent);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets->progress_bar), text);
    
    // Update text view with detailed info
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(widgets->text_view));
    char detail_text[4096];
    sprintf(detail_text, "═══════════════════ ACTIVE USERS ═══════════════════\n");
    
    int user_count = 0;
    for (int i = 0; i < MAX_USERS && user_count < widgets->shm_ptr->control.user_count; i++) {
        if (widgets->shm_ptr->users[i].is_active) {
            sprintf(detail_text + strlen(detail_text), 
                   "User #%d: %s <%s> - Age: %d\n",
                   widgets->shm_ptr->users[i].user_id,
                   widgets->shm_ptr->users[i].name,
                   widgets->shm_ptr->users[i].email,
                   widgets->shm_ptr->users[i].age);
            user_count++;
        }
    }
    
    if (user_count == 0) {
        sprintf(detail_text + strlen(detail_text), "(No active users)\n");
    }
    
    sprintf(detail_text + strlen(detail_text), "\n═══════════════════ RECENT EMAILS ═══════════════════\n");
    
    int email_count = 0;
    for (int i = 0; i < MAX_EMAILS && email_count < 10 && email_count < widgets->shm_ptr->control.email_count; i++) {
        if (widgets->shm_ptr->emails[i].email_id > 0 && !widgets->shm_ptr->emails[i].is_deleted) {
            sprintf(detail_text + strlen(detail_text),
                   "Email #%d: From User#%d → User#%d\n  Subject: %s\n  Status: %s\n",
                   widgets->shm_ptr->emails[i].email_id,
                   widgets->shm_ptr->emails[i].sender_id,
                   widgets->shm_ptr->emails[i].receiver_id,
                   widgets->shm_ptr->emails[i].subject,
                   widgets->shm_ptr->emails[i].is_read ? "Read" : "Unread");
            email_count++;
        }
    }
    
    if (email_count == 0) {
        sprintf(detail_text + strlen(detail_text), "(No emails)\n");
    }
    
    gtk_text_buffer_set_text(buffer, detail_text, -1);
    
    return TRUE;
}

// Clean up on window close
static void on_window_destroy(GtkWidget *widget, gpointer data) {
    (void)widget; // Suppress unused parameter warning
    AppWidgets *widgets = (AppWidgets *)data;
    
    if (widgets->timer_id > 0) {
        g_source_remove(widgets->timer_id);
    }
    
    if (widgets->shm_ptr != NULL) {
        detach_shared_memory(widgets->shm_ptr);
    }
    
    g_free(widgets);
    gtk_main_quit();
}

// Activate callback to create the window
static void activate(GtkApplication *app, gpointer user_data) {
    (void)user_data; // Suppress unused parameter warning
    AppWidgets *widgets = g_new(AppWidgets, 1);
    
    // Attach to shared memory
    widgets->shm_ptr = attach_shared_memory();

    // Create main window
    widgets->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(widgets->window), "Mail System - Shared Memory Monitor");
    gtk_window_set_default_size(GTK_WINDOW(widgets->window), 700, 650);
    gtk_container_set_border_width(GTK_CONTAINER(widgets->window), 15);

    // Create vertical box container
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(widgets->window), vbox);

    // Create title label
    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), 
                        "<span size='xx-large' weight='bold'>📊 Shared Memory Monitor</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title_label, FALSE, FALSE, 5);

    // Status label
    widgets->status_label = gtk_label_new("Initializing...");
    gtk_box_pack_start(GTK_BOX(vbox), widgets->status_label, FALSE, FALSE, 5);

    // Create separator
    GtkWidget *separator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator1, FALSE, FALSE, 5);

    // Create labels for shared memory info
    widgets->shm_id_label = gtk_label_new("Shared Memory ID: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->shm_id_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->shm_id_label, FALSE, FALSE, 3);

    widgets->shm_size_label = gtk_label_new("Memory Size: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->shm_size_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->shm_size_label, FALSE, FALSE, 3);

    // Create separator
    GtkWidget *separator2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator2, FALSE, FALSE, 5);

    // Data section label
    GtkWidget *data_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(data_label), 
                        "<span size='large' weight='bold'>📦 Data Statistics</span>");
    gtk_label_set_xalign(GTK_LABEL(data_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), data_label, FALSE, FALSE, 3);

    widgets->users_label = gtk_label_new("Total Users: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->users_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->users_label, FALSE, FALSE, 3);

    widgets->emails_label = gtk_label_new("Total Emails: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->emails_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->emails_label, FALSE, FALSE, 3);

    widgets->next_user_id_label = gtk_label_new("Next User ID: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->next_user_id_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->next_user_id_label, FALSE, FALSE, 3);

    widgets->next_email_id_label = gtk_label_new("Next Email ID: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->next_email_id_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->next_email_id_label, FALSE, FALSE, 3);

    // Create separator
    GtkWidget *separator3 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator3, FALSE, FALSE, 5);

    // Memory usage section
    GtkWidget *memory_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(memory_label), 
                        "<span size='large' weight='bold'>💾 Memory Usage</span>");
    gtk_label_set_xalign(GTK_LABEL(memory_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), memory_label, FALSE, FALSE, 3);

    widgets->memory_used_label = gtk_label_new("Memory Used: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->memory_used_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->memory_used_label, FALSE, FALSE, 3);

    widgets->memory_total_label = gtk_label_new("Memory Total: --");
    gtk_label_set_xalign(GTK_LABEL(widgets->memory_total_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->memory_total_label, FALSE, FALSE, 3);

    widgets->usage_label = gtk_label_new("Usage: --%");
    gtk_label_set_xalign(GTK_LABEL(widgets->usage_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->usage_label, FALSE, FALSE, 3);

    // Create progress bar
    widgets->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(widgets->progress_bar), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), widgets->progress_bar, FALSE, FALSE, 8);

    // Create separator
    GtkWidget *separator4 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(vbox), separator4, FALSE, FALSE, 5);

    // Details section
    GtkWidget *details_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(details_label), 
                        "<span size='large' weight='bold'>📋 Details</span>");
    gtk_label_set_xalign(GTK_LABEL(details_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), details_label, FALSE, FALSE, 3);

    // Create scrolled window for text view
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrolled_window, -1, 200);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Create text view
    widgets->text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(widgets->text_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(widgets->text_view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(widgets->text_view), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), widgets->text_view);

    // Connect destroy signal
    g_signal_connect(widgets->window, "destroy", G_CALLBACK(on_window_destroy), widgets);

    // Initial update
    update_ram_display(widgets);

    // Set up timer to update every 1 second (1000 milliseconds)
    widgets->timer_id = g_timeout_add(1000, update_ram_display, widgets);

    // Show all widgets
    gtk_widget_show_all(widgets->window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.mailsystem.shmmonitor", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
