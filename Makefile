# Makefile for Mail System with Shared Memory IPC

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = mail_system
OBJS = main.o shared_memory.o database.o user_crud.o email_crud.o mail_functions.o

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)
	@echo "Mail System compiled successfully!"

# Compile main.c
main.o: main.c mail_system.h
	$(CC) $(CFLAGS) -c main.c

# Compile shared_memory.c
shared_memory.o: shared_memory.c mail_system.h
	$(CC) $(CFLAGS) -c shared_memory.c

# Compile database.c
database.o: database.c mail_system.h
	$(CC) $(CFLAGS) -c database.c

# Compile user_crud.c
user_crud.o: user_crud.c mail_system.h
	$(CC) $(CFLAGS) -c user_crud.c

# Compile email_crud.c
email_crud.o: email_crud.c mail_system.h
	$(CC) $(CFLAGS) -c email_crud.c

# Compile mail_functions.c
mail_functions.o: mail_functions.c mail_system.h
	$(CC) $(CFLAGS) -c mail_functions.c

# Clean compiled files
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f *.txt
	@echo "Cleaned object files and executable"

# Clean all including database files
clean-all: clean
	rm -f users.txt emails.txt
	rm -f *_backup_*.txt
	@echo "Cleaned all files including database backups"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Debug version
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release version
release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

# Check for memory leaks with valgrind
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

# Show shared memory segments
show-shm:
	ipcs -m

# Remove all shared memory segments (use with caution)
clean-shm:
	@echo "Removing all shared memory segments..."
	@for id in $$(ipcs -m | grep $(USER) | awk '{print $$2}'); do \
		ipcrm -m $$id; \
	done
	@echo "Shared memory cleaned"

# Create sample data for testing
sample-data: $(TARGET)
	@echo "Creating sample users and emails..."
	@echo -e "1\n1\nJohn Doe\njohn@email.com\n25\n6\n3\njohn@email.com\njane@email.com\nHello Jane\nThis is a test email from John.\n\n10" | ./$(TARGET)

# Install (copy to system directory)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Mail System installed to /usr/local/bin/"

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Mail System uninstalled"

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build the mail system (default)"
	@echo "  clean      - Remove object files and executable"
	@echo "  clean-all  - Remove all files including database"
	@echo "  run        - Build and run the program"
	@echo "  debug      - Build debug version"
	@echo "  release    - Build optimized release version"
	@echo "  memcheck   - Run with valgrind memory checker"
	@echo "  show-shm   - Show current shared memory segments"
	@echo "  clean-shm  - Remove all shared memory segments"
	@echo "  sample-data- Create sample data for testing"
	@echo "  install    - Install to system directory"
	@echo "  uninstall  - Remove from system directory"
	@echo "  help       - Show this help message"

# Phony targets
.PHONY: all clean clean-all run debug release memcheck show-shm clean-shm sample-data install uninstall help