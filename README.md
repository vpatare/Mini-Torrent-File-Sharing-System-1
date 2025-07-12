# Mini Torrent File Sharing System

A peer-to-peer (P2P) file sharing system implemented in C++ that mimics the basic functionality of BitTorrent. This system allows users to share and download files in a distributed manner without relying on a central server for file storage.

## 🚀 Features

### Core Functionality
- **File Sharing**: Upload files to the network and make them available to other peers
- **File Downloading**: Download files from available peers in the network
- **File Removal**: Remove shared files from the network
- **Distributed Architecture**: No central file storage - files are shared directly between peers

### Technical Features
- **SHA1 Hashing**: File integrity verification using cryptographic hashes
- **Chunked Transfer**: Files are transferred in 1500-byte chunks for efficiency
- **Multi-threaded**: Concurrent handling of upload and download operations
- **TCP-based**: Reliable communication using TCP sockets
- **Cross-platform**: Works on Linux, macOS, and other Unix-like systems

## 🏗️ Architecture

The system consists of two main components:

### 1. Tracker Server (`tracker1.cpp`)
- **Role**: Central coordinator that maintains file registry
- **Function**: Maps file hashes to peer locations (IP:port)
- **Storage**: Maintains `seeders.txt` file with file-peer mappings
- **Protocol**: Handles client requests for file discovery

### 2. Peer Client (`peer.cpp`)
- **Dual Role**: Acts as both seeder (uploader) and leecher (downloader)
- **Server Thread**: Runs file server to handle incoming requests
- **Client Thread**: Connects to tracker for file registration and discovery
- **P2P Communication**: Direct peer-to-peer file transfers

```
Peer A (Seeder) ←→ Tracker Server ←→ Peer B (Leecher)
     ↑                                    ↑
  File Server                         File Client
```

## 📋 Prerequisites

### System Requirements
- **Operating System**: Linux, macOS, or other Unix-like systems
- **Compiler**: GCC/G++ with C++17 support
- **Libraries**: OpenSSL development library
- **Network**: TCP/IP connectivity

### Dependencies Installation

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install g++ libssl-dev make
```

#### macOS (using Homebrew):
```bash
brew install openssl
```

#### CentOS/RHEL:
```bash
sudo yum install gcc-c++ openssl-devel make
```

## 🔧 Installation & Build

1. **Clone the repository:**
```bash
git clone <repository-url>
cd Mini-Torrent-File-Sharing-System
```

2. **Build the project:**
```bash
make
```

This will create two executables:
- `tracker1` - The tracker server
- `peer` - The peer client

## 🚀 Usage

### Starting the Tracker Server

```bash
./tracker1 <tracker_ip:port>
```

**Example:**
```bash
./tracker1 127.0.0.1:4444
```

The tracker will:
- Start listening on the specified IP and port
- Create/load `seeders.txt` file for storing file-peer mappings
- Accept connections from peers

### Starting a Peer Client

```bash
./peer <peer_ip:port> <tracker1_ip:port> <tracker2_ip:port>
```

**Example:**
```bash
./peer 127.0.0.1:5555 127.0.0.1:4444 127.0.0.1:4445
```

**Parameters:**
- `peer_ip:port`: Your peer's IP address and port for file serving
- `tracker1_ip:port`: Primary tracker server address
- `tracker2_ip:port`: Secondary tracker server address (for redundancy)

## 📖 Available Commands

Once the peer client is running, you can use the following commands:

### 1. Share a File
```bash
share <file_path> <mtorrent_destination>
```

**Description**: Upload a file to the network and create a `.mtorrent` file
- `file_path`: Path to the file you want to share
- `mtorrent_destination`: Where to save the generated `.mtorrent` file

**Example:**
```bash
share /home/user/document.pdf /home/user/document.pdf.mtorrent
```

**What happens:**
1. Creates a `.mtorrent` file with file metadata and SHA1 hash
2. Registers the file with the tracker server
3. Makes your peer available as a seeder for this file

### 2. Download a File
```bash
get <mtorrent_file> <save_location>
```

**Description**: Download a file using its `.mtorrent` file
- `mtorrent_file`: Path to the `.mtorrent` file
- `save_location`: Where to save the downloaded file

**Example:**
```bash
get document.pdf.mtorrent /home/user/downloaded_document.pdf
```

**What happens:**
1. Reads file hash from the `.mtorrent` file
2. Queries tracker for available seeders
3. Connects directly to seeders for file download
4. Downloads file in chunks and reconstructs it

### 3. Remove a Shared File
```bash
remove <mtorrent_file>
```

**Description**: Remove a shared file from the network
- `mtorrent_file`: Path to the `.mtorrent` file of the file to remove

**Example:**
```bash
remove document.pdf.mtorrent
```

**What happens:**
1. Unregisters the file from the tracker
2. Removes the file from local storage
3. Updates the seeder list

### 4. Exit the Application
```bash
exit
```

**Description**: Gracefully disconnect from the network and exit

## 🔍 File Structure

### `.mtorrent` File Format
The system creates `.mtorrent` files that contain:
```
tracker1_ip:port
tracker2_ip:port
absolute_file_path
file_size
chunk_hash_1
chunk_hash_2
...
chunk_hash_n
file_hash_of_hash
```

### `seeders.txt` File Format
The tracker maintains a file with entries like:
```
file_path file_hash peer_ip:port
```

## 🔧 Technical Details

### Network Protocol
- **Transport**: TCP sockets
- **Chunk Size**: 1500 bytes
- **Hash Algorithm**: SHA1
- **Encoding**: Binary for file transfer, text for metadata

### File Transfer Process
1. **Registration**: Peer registers file with tracker using file hash
2. **Discovery**: Downloader queries tracker for file location
3. **Connection**: Direct P2P connection established
4. **Transfer**: File transferred in chunks with integrity verification
5. **Reconstruction**: File reassembled from chunks

### Error Handling
- Network connection failures
- File not found errors
- Invalid `.mtorrent` files
- Peer disconnection handling

## 🐛 Troubleshooting

### Common Issues

1. **"FILE NOT FOUND"**
   - Ensure the file path is correct and accessible
   - Check file permissions

2. **"MTORRENT_FILE_NOT_FOUND"**
   - Verify the `.mtorrent` file exists and is readable
   - Check file path in the `.mtorrent` file

3. **"FILE NOT FOUND" (during download)**
   - No seeders available for the requested file
   - File may have been removed from the network

4. **Connection Errors**
   - Check if tracker server is running
   - Verify IP addresses and ports
   - Ensure firewall allows connections

### Debug Information
The system provides debug output including:
- Connection status messages
- File transfer progress
- Peer discovery information
- Error messages

## 🔒 Security Considerations

### Current Limitations
- No peer authentication
- No encryption for file transfers
- No protection against malicious files

### Recommendations
- Use in trusted networks only
- Verify downloaded files independently
- Consider implementing peer authentication

## 📈 Performance

### Factors Affecting Performance
- **Network Bandwidth**: Higher bandwidth = faster transfers
- **Number of Seeders**: More seeders = faster downloads
- **File Size**: Larger files take longer to transfer
- **Network Latency**: Lower latency = better performance

### Optimization Tips
- Use multiple peers for large files
- Ensure stable network connections
- Monitor system resources during transfers

## 🤝 Contributing

To contribute to this project:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## 📄 License

This project is open source. Please check the license file for details.

## 📞 Support

For issues and questions:
1. Check the troubleshooting section
2. Review the code comments
3. Create an issue in the repository

---

**Note**: This is a simplified implementation for educational purposes. For production use, consider implementing additional security features and error handling.
