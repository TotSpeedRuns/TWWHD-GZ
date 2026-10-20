#include "contents.hpp"

#include <filesystem>

#include <nuspack/fst/FSTEntries.hpp>
#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <utility/math.hpp>
#include <utility/string.hpp>

#include <gui/desktop/update_dialog_header.hpp>

using eType = Utility::Endian::Type;



uint64_t Content::GetOffsetForFileAndIncrease(const FSTEntry& entry) {
    const uint64_t oldOffset = curFileOffset;
    curFileOffset += roundUp(std::get<FSTEntry::FileEntry>(entry.entry).fileSize, ALIGNMENT_IN_CONTENT);

    return oldOffset;
}

void Content::Update(const std::vector<FSTEntry*>& entries) {
    if(entries.size() > 0) {
        this->entries = entries;
    }
}

static constexpr std::streamsize MAX_SSTREAM_SIZE = 1024 * 1024 * 1;
static constexpr size_t READ_BUFFER_SIZE = 1024 * 1024 * 1;
static char readBuffer[READ_BUFFER_SIZE];

std::variant<std::stringstream, std::ifstream> Content::PackDecrypted()
{
    const fspath& tmpPath = Utility::get_temp_dir() / (Utility::Str::intToHex(id, 8, false) + ".dec");
    // Stream directly to SD. Avoid accumulating a whole BFRES/audio file in RAM.
    std::ofstream outFile(tmpPath, std::ios::binary);
    if (!outFile) throw std::runtime_error("Could not open package scratch file");

    size_t count = 0;
    
    for (FSTEntry* pEntry : entries)
    {
        count++;
        if (pEntry->isFile())
        {
            const FSTEntry::FileEntry& entry = std::get<FSTEntry::FileEntry>(pEntry->entry);

            std::ifstream input(pEntry->path, std::ios::binary);
            if (!input) throw std::runtime_error("Could not read copied game file");
            while(input) {
                input.read(readBuffer, READ_BUFFER_SIZE);
                outFile.write(readBuffer, input.gcount());
                if (!outFile) throw std::runtime_error("SD scratch write failed");
            }
            if (!input.eof()) throw std::runtime_error("Copied game read failed");

            uint64_t alignedFileSize = roundUp(entry.fileSize, ALIGNMENT_IN_CONTENT);

            uint64_t padding = alignedFileSize - entry.fileSize;
            Utility::seek(outFile, padding, std::ios::cur);
            
            // Update progress dialog
            if (entries.size() > 1000)
            {
                UPDATE_DIALOG_VALUE(99 + (int)(((float) count / (float) entries.size()) * 50.0f))
            }
        }
        else
        {
            //Console.WriteLine($"[{cnt_file}/{totalCount}] Wrote folder: \"{entry.filename}\"");
        }
    }

    outFile.close();
    if (!outFile) throw std::runtime_error("SD scratch close failed");
    return std::ifstream(tmpPath, std::ios::binary);
}

void Content::PackContentToFile(const fspath& outputDir, Encryption& encryption) {
    // At first we need to create the decrypted file.
    auto decryptedStream = PackDecrypted();

    std::istream& file = decryptedStream.index() == 0 ? std::get<0>(decryptedStream) : static_cast<std::istream&>(std::get<1>(decryptedStream));

    // Calculates the hashes for the decrypted content. If the content is not hashed,
    // only the hash of the decrypted file will be calculated
    ContentHashes contentHashes(file, isHashed());

    if (contentHashes.h3Hashes.size() > 0)
    {
        const fspath h3Path = outputDir / (Utility::Str::intToHex(id, 8, false) + ".h3");
    
        std::ofstream hashOut(h3Path, std::ios::binary);
        contentHashes.SaveH3ToFile(hashOut);
    }
    hash = contentHashes.TMDHash;
    
    const fspath outputFilePath = outputDir / (Utility::Str::intToHex(id, 8, false) + ".app");
    std::ofstream output(outputFilePath, std::ios::binary);
    if (!output) throw std::runtime_error("Cannot create encrypted package file");
    file.clear();
    size = PackEncrypted(file, output, contentHashes, encryption);
    output.close();
    if (!output) throw std::runtime_error("Encrypted package write failed");
    
    // delete derypted file, we're done with it
    if(decryptedStream.index() == 1) {
        std::get<1>(decryptedStream).close();
        std::filesystem::remove(Utility::get_temp_dir() / (Utility::Str::intToHex(id, 8, false) + ".dec"));
    }
}

uint64_t Content::PackEncrypted(std::istream& input, std::ostream& output, ContentHashes& hashes, Encryption& encryption) {
    if (isHashed())
    {
        const uint64_t size = input.seekg(0, std::ios::end).tellg();
        input.seekg(0, std::ios::beg);
        encryption.EncryptFileHashed(input, output, size, *this, hashes);
    }
    else
    {
        encryption.EncryptFileWithPadding(input, id, output, PAD_LEN);
    }

    return output.tellp(); // should always be at the end?
}

uint64_t Content::writeFSTContentHeader(std::ostream& out, const uint64_t& oldOffset) {
    uint8_t unknown;
    uint64_t content_offset = oldOffset;
    uint64_t fst_content_size = size / PAD_LEN;
    uint64_t fst_content_size_written = fst_content_size;

    if (isHashed())
    {
        unknown = 2;
        fst_content_size_written -= ((fst_content_size / 64) + 1) * 2;
        if (fst_content_size_written < 0) fst_content_size_written = 0;
    }
    else
    {
        unknown = 1;
    }

    if (isFSTContent)
    {
        unknown = 0;
        if (fst_content_size == 1) fst_content_size = 0;

        content_offset += fst_content_size + 2;
    }
    else
    {
        content_offset += fst_content_size;
    }

    const uint32_t offs = Utility::Endian::toPlatform(eType::Big, static_cast<uint32_t>(oldOffset));
    const uint32_t written = Utility::Endian::toPlatform(eType::Big, static_cast<uint32_t>(fst_content_size_written));
    const uint64_t tID = Utility::Endian::toPlatform(eType::Big, parentTitleID);
    const uint32_t gID = Utility::Endian::toPlatform(eType::Big, groupID);

    const std::streamoff& start = out.tellp();
    out.write(reinterpret_cast<const char*>(&offs), sizeof(offs));
    out.write(reinterpret_cast<const char*>(&written), sizeof(written));
    out.write(reinterpret_cast<const char*>(&tID), sizeof(tID));
    out.write(reinterpret_cast<const char*>(&gID), sizeof(gID));
    out.write(reinterpret_cast<const char*>(&unknown), sizeof(unknown));
    Utility::seek(out, start + 0x20, std::ios::beg); // adds padding

    return content_offset;
}

void Content::writeToStream(std::ostream& out) {
    const auto ID_ = Utility::Endian::toPlatform(eType::Big, id);
    const auto idx_ = Utility::Endian::toPlatform(eType::Big, index);
    const auto type_ = Utility::Endian::toPlatform(eType::Big, type);
    const auto size_ = Utility::Endian::toPlatform(eType::Big, size);

    out.write(reinterpret_cast<const char*>(&ID_), sizeof(ID_));
    out.write(reinterpret_cast<const char*>(&idx_), sizeof(idx_));
    out.write(reinterpret_cast<const char*>(&type_), sizeof(type_));
    out.write(reinterpret_cast<const char*>(&size_), sizeof(size_));
    out.write(reinterpret_cast<const char*>(&hash[0]), sizeof(hash));
    Utility::seek(out, 0xC, std::ios::cur); // padding
}



Content* Contents::GetNewContent(const ContentDetails& details, const bool& isFSTContent) {
    Content& newContent = contents.emplace_back(contents.size(), contents.size(), details.entriesFlag, details.groupID, details.parentID, details.isHashed, isFSTContent);
    return &newContent;
}

void Contents::DeleteContent(const size_t& idx) {
    std::list<Content>::iterator it = contents.begin();
    std::advance(it, idx);
    contents.erase(it);
}

void Contents::ResetFileOffsets() {
    for(Content& content : contents) {
        content.ResetFileOffset();
    }
}

void Contents::Update(const FSTEntries& entries) {
    for(Content& content : contents) {
        content.Update(entries.GetFSTEntriesByContent(content));
    }
}

void Contents::PackContents(const fspath& out, Encryption& encryption) {
    for(Content& content : contents) {
        if(!content.isFSTContent) {
            content.PackContentToFile(out, encryption);
        }
    }
}

void Contents::writeFSTContentHeader(std::ostream& out) {
    uint64_t contentOffset = 0;
    for(Content& content : contents) {
        contentOffset = content.writeFSTContentHeader(out, contentOffset);
    }
}

void Contents::writeToStream(std::ostream& out) {
    std::vector<Content*> sortedContents(contents.size()); // sorted by index
    for(Content& content : contents) {
        sortedContents[content.index] = &content;
    }

    for(Content* content : sortedContents) {
        content->writeToStream(out);
    }
}
