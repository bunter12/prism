#include "mapped_file.hpp"
#include <fcntl.h>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iostream>

namespace prism::core{
    MappedFile::MappedFile(const std::filesystem::path& path) {
        int fd = open(path.c_str(), O_RDONLY);

        if (fd < 0){
            throw std::system_error(errno, std::generic_category(), "Failed to open file: " + path.string());
        }

        struct FdGuard {
            int fd;
            ~FdGuard() { if (fd >= 0) close(fd); }
        } fd_guard{fd};

        struct stat sb{};

        if(fstat(fd, &sb) == -1){
            throw std::system_error(errno, std::generic_category(), "Failed to get file size: " + path.string());
        }

        if(!S_ISREG(sb.st_mode)){
            throw std::system_error(EISDIR, std::generic_category(), "Target path is not a regular file: " + path.string());
        }

        if(sb.st_size == 0){
            data_ = nullptr;
            size_ = 0;
            return;
        }

        size_ = static_cast<size_t>(sb.st_size);

        void* mapped_memory = mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0);
        if(mapped_memory == MAP_FAILED){
            throw std::system_error(errno, std::generic_category(), "Failed to map file: " + path.string());
        }

        data_ = static_cast<const uint8_t*>(mapped_memory);

        #if defined(MADV_SEQUENTIAL)
        madvise(mapped_memory, size_, MADV_SEQUENTIAL);
        #endif
    }

    MappedFile::~MappedFile() {
        cleanup();
    }
    
    void MappedFile::cleanup() noexcept {
        if (data_ != nullptr && size_ > 0) {
            munmap(const_cast<void*>(static_cast<const void*>(data_)), size_);
            data_ = nullptr;
            size_ = 0;
        }
    }

    MappedFile::MappedFile(MappedFile&& other) noexcept 
    : data_(std::exchange(other.data_, nullptr)),
      size_(std::exchange(other.size_, 0)) {}

    MappedFile& MappedFile::operator=(MappedFile&& other) noexcept {
        if (this != &other) {
            cleanup();
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }
    
    std::span<const uint8_t> MappedFile::span() const noexcept {
        return {data_, size_};
    }

    const uint8_t* MappedFile::data() noexcept {
        return data_;
    }

    size_t MappedFile::size() const noexcept {
        return size_;
    }  

    bool MappedFile::empty() const noexcept {
        return size_ == 0;
    }

    bool MappedFile::is_mapped() const noexcept {
        return data_ != nullptr;
    }

} // namespace prism::core