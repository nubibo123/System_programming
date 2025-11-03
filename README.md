# Mail System với Shared Memory IPC

Một hệ thống email đơn giản được xây dựng bằng C sử dụng Shared Memory IPC cho việc chia sẻ dữ liệu giữa các process.

## Tính năng chính

### 1. Shared Memory IPC
- Sử dụng `sys/ipc.h` và `sys/shm.h` để tạo và quản lý shared memory
- Chia sẻ dữ liệu giữa nhiều process
- Tự động tạo, attach, detach và cleanup shared memory

### 2. User Management (CRUD)
- **Create**: Đăng ký user mới với thông tin (ID, name, email, age)
- **Read**: Xem thông tin user, tìm kiếm theo email
- **Update**: Cập nhật thông tin user
- **Delete**: Xóa user khỏi hệ thống

### 3. Email System
- **Compose**: Soạn và gửi email mới
- **Read**: Đọc email theo ID
- **View Sent**: Xem danh sách email đã gửi
- **View Received**: Xem danh sách email đã nhận
- **Search**: Tìm kiếm email theo nội dung
- **Reply**: Trả lời email
- **Delete**: Xóa email

### 4. Database Management
- Lưu trữ dữ liệu vào file `.dat` 
- Tự động backup và restore
- Validate tính toàn vẹn dữ liệu
- Import/Export dữ liệu

## Cấu trúc Project

```
├── mail_system.h       # Header file với các struct và prototypes
├── main.c             # Chương trình chính với menu
├── shared_memory.c    # Functions quản lý shared memory IPC
├── database.c         # Functions lưu/đọc dữ liệu từ file
├── user_crud.c        # CRUD operations cho users
├── email_crud.c       # CRUD operations cho emails
├── mail_functions.c   # Functions chức năng email system
├── Makefile          # Build configuration
└── README.md         # Documentation
```

## Compilation và Usage

### Compile
```bash
make                 # Build chương trình
make debug          # Build với debug info
make release        # Build phiên bản tối ưu
```

### Run
```bash
make run            # Build và chạy
./mail_system       # Chạy trực tiếp
```

### Cleanup
```bash
make clean          # Xóa object files
make clean-all      # Xóa tất cả files bao gồm database
make clean-shm      # Xóa shared memory segments
```

### Testing
```bash
make sample-data    # Tạo dữ liệu mẫu để test
make memcheck       # Kiểm tra memory leaks với valgrind
```

## Cách sử dụng

### 1. Khởi động hệ thống
- Chạy chương trình: `./mail_system`
- Hệ thống sẽ tự động tạo và khởi tạo shared memory
- Load dữ liệu từ file database nếu có

### 2. Đăng ký User
```
Main Menu → 1. User Management → 1. Register New User
```
Nhập thông tin:
- Name: Tên người dùng
- Email: Địa chỉ email (unique)
- Age: Tuổi

### 3. Soạn Email
```
Main Menu → 3. Compose New Email
```
Nhập:
- Sender email: Email người gửi (phải đã đăng ký)
- Receiver email: Email người nhận (phải đã đăng ký)
- Subject: Tiêu đề email
- Content: Nội dung (nhập xong ấn Enter 2 lần)

### 4. Xem Email
- **Sent emails**: `Main Menu → 5. View Sent Emails`
- **Received emails**: `Main Menu → 6. View Received Emails`
- **Read specific email**: `Main Menu → 4. Read Email`

### 5. Tìm kiếm
- **Search emails**: `Main Menu → 7. Search Emails`
- **Search users**: `Main Menu → 1. User Management → 3. Search Users`

## Shared Memory Architecture

### Key Components
- **SHM_KEY_USERS (1234)**: Key cho shared memory segment
- **SharedMemoryData**: Main structure chứa:
  - `ControlData`: Metadata (counters, IDs)
  - `User users[MAX_USERS]`: Array of users
  - `Email emails[MAX_EMAILS]`: Array of emails

### Memory Layout
```c
typedef struct {
    ControlData control;        // Control information
    User users[100];           // User array
    Email emails[1000];        // Email array  
} SharedMemoryData;
```

### IPC Functions
- `shmget()`: Tạo/lấy shared memory segment
- `shmat()`: Attach process vào shared memory
- `shmdt()`: Detach process khỏi shared memory
- `shmctl()`: Control/destroy shared memory

## Database Files

- `users.dat`: Lưu trữ thông tin users
- `emails.dat`: Lưu trữ emails
- `*_backup_*.dat`: Các file backup tự động

## System Requirements

- Linux/Unix system
- GCC compiler
- POSIX shared memory support
- Make utility

## Advanced Features

### 1. Multi-Process Support
- Nhiều process có thể truy cập cùng lúc
- Dữ liệu được đồng bộ qua shared memory
- Signal handling để cleanup khi exit

### 2. Data Persistence
- Auto-save khi thay đổi dữ liệu
- Auto-load khi khởi động
- Backup/restore functionality

### 3. Error Handling
- Validation input data
- Memory management
- Database integrity checks

## Troubleshooting

### Common Issues
1. **Permission denied**: Chạy với quyền phù hợp
2. **Shared memory full**: Sử dụng `make clean-shm`
3. **Segmentation fault**: Kiểm tra với `make memcheck`

### Debug Commands
```bash
ipcs -m                 # Xem shared memory segments
ipcrm -m <shmid>       # Xóa specific shared memory
make show-shm          # Show shared memory status
```

## Contribution

Để contribute code:
1. Fork repository
2. Tạo feature branch
3. Implement changes
4. Test thoroughly
5. Submit pull request

## License

This project is for educational purposes.