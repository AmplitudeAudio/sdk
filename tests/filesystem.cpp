// Copyright (c) 2021-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <catch2/catch_test_macros.hpp>

#include <SparkyStudios/Audio/Amplitude/Amplitude.h>

using namespace SparkyStudios::Audio::Amplitude;

TEST_CASE("DiskFileSystem Tests", "[filesystem][amplitude]")
{
    DiskFileSystem fileSystem;
    fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

    const auto& cp = std::filesystem::current_path() / AM_OS_STRING("samples/assets");

    SECTION("can open filesystem")
    {
        fileSystem.StartOpenFileSystem();
        REQUIRE(fileSystem.TryFinalizeOpenFileSystem());
    }

    SECTION("can sets the base path")
    {
        REQUIRE(fileSystem.GetBasePath() == cp);
    }

    SECTION("can resolve paths")
    {
        REQUIRE(
            fileSystem.ResolvePath(AM_OS_STRING("sounds/test.wav")) ==
            (cp / AM_OS_STRING("sounds/test.wav")).lexically_normal().make_preferred().native());
        REQUIRE(
            fileSystem.ResolvePath(AM_OS_STRING("../../samples/assets/sounds/../test.wav")) ==
            (cp / AM_OS_STRING("test.wav")).lexically_normal().make_preferred().native());
        REQUIRE(
            fileSystem.ResolvePath(AM_OS_STRING("./sounds/../sounds/./test.wav")) ==
            (cp / AM_OS_STRING("sounds/test.wav")).lexically_normal().make_preferred().native());
    }

    SECTION("can check if files exists")
    {
        REQUIRE(fileSystem.Exists(AM_OS_STRING("tests.config.amconfig")));
        REQUIRE_FALSE(fileSystem.Exists(AM_OS_STRING("some_random_file.ext")));
    }

    SECTION("can detect if a file is a directory")
    {
        REQUIRE(fileSystem.IsDirectory(AM_OS_STRING("sounds")));
        REQUIRE_FALSE(fileSystem.IsDirectory(AM_OS_STRING("tests.config.amconfig")));
    }

    SECTION("can join paths")
    {
        REQUIRE(fileSystem.Join({}).empty());
        REQUIRE(
            fileSystem.Join({ AM_OS_STRING("sounds"), AM_OS_STRING("test.wav") }) ==
            std::filesystem::path(AM_OS_STRING("sounds/test.wav")).lexically_normal().make_preferred().native());
        REQUIRE(
            fileSystem.Join({ AM_OS_STRING("../sample_project/sounds/../test.wav") }) ==
            std::filesystem::path(AM_OS_STRING("../sample_project/test.wav")).lexically_normal().make_preferred().native());
        REQUIRE(
            fileSystem.Join({ AM_OS_STRING("./sounds"), AM_OS_STRING("../sounds/"), AM_OS_STRING("./test.wav") }) ==
            std::filesystem::path(AM_OS_STRING("sounds/test.wav")).lexically_normal().make_preferred().native());
    }

    SECTION("can open files")
    {
        REQUIRE(fileSystem.OpenFile(AM_OS_STRING("tests.config.amconfig"), eFileOpenMode_Read)->IsValid());
        REQUIRE_FALSE(fileSystem.OpenFile(AM_OS_STRING("some_random_file.ext"), eFileOpenMode_Read)->IsValid());
    }

    SECTION("can close filesystem")
    {
        fileSystem.StartCloseFileSystem();
        REQUIRE(fileSystem.TryFinalizeCloseFileSystem());
    }
}

TEST_CASE("DiskFileSystem DiskFile Tests", "[filesystem][amplitude]")
{
    DiskFileSystem fileSystem;
    fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

    const auto& file = fileSystem.OpenFile(AM_OS_STRING("test_data/diskfile_read_test.txt"), eFileOpenMode_Read);

    SECTION("can open files")
    {
        REQUIRE(file->IsValid());
    }

    SECTION("can return the correct file path")
    {
        REQUIRE(file->GetPath() == fileSystem.ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt")));
    }

    SECTION("can return the correct file size")
    {
        REQUIRE(file->Length() == 2);
    }

    SECTION("can read the file")
    {
        REQUIRE(file->Read8() == 'O');
        REQUIRE(file->Read8() == 'K');
    }

    SECTION("can seek the file")
    {
        file->Seek(1, eFileSeekOrigin_Start);
        REQUIRE(file->Position() == 1);
        REQUIRE(file->Read8() == 'K');
        file->Seek(-2, eFileSeekOrigin_End);
        REQUIRE(file->Position() == 0);
        REQUIRE(file->Read8() == 'O');
        file->Seek(-1, eFileSeekOrigin_Current);
        REQUIRE(file->Position() == 0);
        REQUIRE(file->Read8() == 'O');
    }

    SECTION("can read the entire file")
    {
        file->Seek(0);
        auto* content = static_cast<AmUInt8Buffer>(ammalloc(2));
        REQUIRE(file->Read(content, file->Length()) == file->Length());
        REQUIRE(content[0] == 'O');
        REQUIRE(content[1] == 'K');
        REQUIRE(file->Position() == file->Length());
        REQUIRE(file->Eof());
        amfree(content);
    }

    SECTION("can close files")
    {
        static_cast<DiskFile*>(file.get())->Close();
        REQUIRE_FALSE(file->IsValid());
        REQUIRE(file->Length() == 0);
        REQUIRE(file->GetPtr() == nullptr);
    }
}

TEST_CASE("Native DiskFile Tests", "[filesystem][amplitude]")
{
    DiskFileSystem fileSystem;
    fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

    DiskFile file(fileSystem.ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt")), eFileOpenMode_ReadWrite, eFileOpenKind_Binary);

    SECTION("can open files")
    {
        REQUIRE(file.IsValid());
    }

    SECTION("cannot open empty paths")
    {
        DiskFile temp;
        REQUIRE(temp.Open(AM_OS_STRING(""), eFileOpenMode_ReadWrite, eFileOpenKind_Binary) == eErrorCode_InvalidParameter);
    }

    SECTION("can return the correct file path")
    {
        REQUIRE(file.GetPath() == fileSystem.ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt")));
    }

    SECTION("can return the correct file size")
    {
        REQUIRE(file.Length() == 0);
    }

    SECTION("can write the file")
    {
        file.Seek(0, eFileSeekOrigin_Start);
        REQUIRE(file.Write8('K') == 1);
        REQUIRE(file.Write8('O') == 1);

        SECTION("can seek the file")
        {
            file.Seek(1, eFileSeekOrigin_Start);
            REQUIRE(file.Position() == 1);
            REQUIRE(file.Read8() == 'O');
        }

        SECTION("can read the entire file")
        {
            file.Seek(0, eFileSeekOrigin_Start);
            auto* content = static_cast<AmUInt8Buffer>(ammalloc(2));
            REQUIRE(file.Read(content, 2) == 2);
            REQUIRE(content[0] == 'K');
            REQUIRE(content[1] == 'O');
            REQUIRE(file.Position() == file.Length());
            REQUIRE(file.Eof());
            amfree(content);
        }

        SECTION("can write the entire file")
        {
            file.Seek(0, eFileSeekOrigin_Start);
            auto* content = static_cast<AmUInt8Buffer>(ammalloc(2));
            content[0] = 'O';
            content[1] = 'K';
            REQUIRE(file.Write(content, 2) == 2);
            REQUIRE(file.Position() == file.Length());
            REQUIRE(file.Eof());
            amfree(content);
        }
    }

    SECTION("can close files")
    {
        file.Seek(0, eFileSeekOrigin_Start);
        REQUIRE(file.Write8('O') == 1);
        REQUIRE(file.Write8('K') == 1);

        file.Close();
        REQUIRE_FALSE(file.IsValid());
        REQUIRE(file.Length() == 0);
        REQUIRE(file.GetPtr() == nullptr);
    }
}

TEST_CASE("MemoryFile Tests", "[filesystem][amplitude]")
{
    MemoryFile file;
    file.Open(2);

    file.Seek(0, eFileSeekOrigin_Start);
    file.Write8('O');
    file.Write8('K');
    file.Seek(0, eFileSeekOrigin_Start);

    SECTION("can open files")
    {
        DiskFileSystem fileSystem;
        fileSystem.SetBasePath(AM_OS_STRING("./samples/assets"));

        REQUIRE(file.IsValid());

        char ok[] = "OK";

        file.Close();
        REQUIRE(file.OpenMem(nullptr, 2) == eErrorCode_InvalidParameter);
        REQUIRE_FALSE(file.IsValid());
        REQUIRE(file.OpenMem(reinterpret_cast<AmConstUInt8Buffer>(ok), 2, false, false) == eErrorCode_Success);
        REQUIRE(file.IsValid());
        REQUIRE(file.GetPtr() == ok);

        file.Close();
        REQUIRE(ok[0] == 'O');
        REQUIRE(ok[1] == 'K');
        REQUIRE(file.OpenMem(reinterpret_cast<AmConstUInt8Buffer>(ok), 2, true, true) == eErrorCode_Success);
        REQUIRE(file.IsValid());
        REQUIRE(file.Read(reinterpret_cast<AmUInt8Buffer>(ok), 2) == 2);
        REQUIRE(ok[0] == 'O');
        REQUIRE(ok[1] == 'K');

        file.Close();
        REQUIRE(file.OpenToMem("") == eErrorCode_InvalidParameter);
        REQUIRE_FALSE(file.IsValid());
        REQUIRE(file.OpenToMem(fileSystem.ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt"))) == eErrorCode_Success);
        REQUIRE(file.IsValid());
        REQUIRE(file.Read(reinterpret_cast<AmUInt8Buffer>(ok), 2) == 2);
        REQUIRE(ok[0] == 'O');
        REQUIRE(ok[1] == 'K');

        file.Close();
        DiskFile df(fileSystem.ResolvePath(AM_OS_STRING("test_data/diskfile_read_test.txt")), eFileOpenMode_Read, eFileOpenKind_Binary);
        REQUIRE(file.OpenFileToMem(nullptr) == eErrorCode_InvalidParameter);
        REQUIRE_FALSE(file.IsValid());
        REQUIRE(file.OpenFileToMem(&df) == eErrorCode_Success);
        REQUIRE(file.IsValid());
        REQUIRE(file.Read(reinterpret_cast<AmUInt8Buffer>(ok), 2) == 2);
        REQUIRE(ok[0] == 'O');
        REQUIRE(ok[1] == 'K');
    }

    SECTION("can return the correct file path")
    {
        REQUIRE(file.GetPath().empty());
    }

    SECTION("can return the correct file size")
    {
        REQUIRE(file.Length() == 2);
    }

    SECTION("can read the file")
    {
        REQUIRE(file.Read8() == 'O');
        REQUIRE(file.Read8() == 'K');
    }

    SECTION("can seek the file")
    {
        file.Seek(1, eFileSeekOrigin_Start);
        REQUIRE(file.Position() == 1);
        REQUIRE(file.Read8() == 'K');
        file.Seek(-2, eFileSeekOrigin_End);
        REQUIRE(file.Position() == 0);
        REQUIRE(file.Read8() == 'O');
        file.Seek(-1, eFileSeekOrigin_Current);
        REQUIRE(file.Position() == 0);
        REQUIRE(file.Read8() == 'O');
        file.Seek(1234, eFileSeekOrigin_Start);
        REQUIRE(file.Position() == 1);
        REQUIRE(file.Read8() == 'K');
    }

    SECTION("can read the entire file")
    {
        file.Seek(0, eFileSeekOrigin_Start);
        auto* content = static_cast<AmUInt8Buffer>(ammalloc(2));
        REQUIRE(file.Read(content, 2) == 2);
        REQUIRE(content[0] == 'O');
        REQUIRE(content[1] == 'K');
        REQUIRE(file.Position() == file.Length());
        REQUIRE(file.Eof());
        amfree(content);
    }

    SECTION("can close files")
    {
        file.Close();
        REQUIRE_FALSE(file.IsValid());
        REQUIRE(file.GetPtr() == nullptr);
    }
}

TEST_CASE("PackageFileSystem Tests", "[filesystem][amplitude]")
{
    PackageFileSystem fileSystem;

    SECTION("can resolve paths")
    {
        REQUIRE(fileSystem.ResolvePath(AM_OS_STRING("sounds/test.wav")) == AM_OS_STRING("sounds/test.wav"));
        REQUIRE(
            fileSystem.ResolvePath(AM_OS_STRING("../../samples/assets/sounds/../test.wav")) ==
            AM_OS_STRING("../../samples/assets/test.wav"));
        REQUIRE(fileSystem.ResolvePath(AM_OS_STRING("./sounds/../sounds/./test.wav")) == AM_OS_STRING("sounds/test.wav"));
    }

    SECTION("cannot detect directories")
    {
        REQUIRE_FALSE(fileSystem.IsDirectory(AM_OS_STRING("sounds")));
        REQUIRE_FALSE(fileSystem.IsDirectory(AM_OS_STRING("tests.config.amconfig")));
    }

    SECTION("can join paths")
    {
        REQUIRE(
            fileSystem.Join({ AM_OS_STRING("sounds"), AM_OS_STRING("test.wav") }) ==
            std::filesystem::path(AM_OS_STRING("sounds/test.wav")).lexically_normal().make_preferred().native());
        REQUIRE(
            fileSystem.Join({ AM_OS_STRING("../sample_project/sounds/../test.wav") }) ==
            std::filesystem::path(AM_OS_STRING("../sample_project/test.wav")).lexically_normal().make_preferred().native());
        REQUIRE(
            fileSystem.Join({ AM_OS_STRING("./sounds"), AM_OS_STRING("../sounds/"), AM_OS_STRING("./test.wav") }) ==
            std::filesystem::path(AM_OS_STRING("sounds/test.wav")).lexically_normal().make_preferred().native());
    }

    SECTION("cannot use an initialized filesystem")
    {
        SECTION("cannot check if files exists")
        {
            REQUIRE_FALSE(fileSystem.Exists(AM_OS_STRING("tests.config.amconfig")));
            REQUIRE_FALSE(fileSystem.Exists(AM_OS_STRING("some_random_file.ext")));
        }

        SECTION("cannot open files")
        {
            REQUIRE(fileSystem.OpenFile(AM_OS_STRING("tests.config.amconfig")) == nullptr);
            REQUIRE(fileSystem.OpenFile(AM_OS_STRING("some_random_file.ext")) == nullptr);
        }
    }

    SECTION("cannot load an invalid package")
    {
        fileSystem.SetBasePath(AM_OS_STRING("./samples/invalid.ampk"));

        fileSystem.StartOpenFileSystem();
        while (!fileSystem.TryFinalizeOpenFileSystem())
            Thread::Sleep(1);

        REQUIRE_FALSE(fileSystem.IsValid());
    }

    SECTION("can use an initialized filesystem")
    {
        PackageFileSystem fs;
        fs.SetBasePath(AM_OS_STRING("./samples/assets.ampk"));

        fs.StartOpenFileSystem();
        while (!fs.TryFinalizeOpenFileSystem())
            Thread::Sleep(1);

        SECTION("can sets the base path")
        {
            REQUIRE(fs.GetBasePath() == std::filesystem::current_path() / AM_OS_STRING("samples/assets.ampk"));
        }

        SECTION("can check if files exists")
        {
            REQUIRE(fs.Exists(AM_OS_STRING("tests.config.amconfig")));
            REQUIRE_FALSE(fs.Exists(AM_OS_STRING("some_random_file.ext")));
        }

        SECTION("can open files")
        {
            REQUIRE(fs.OpenFile(AM_OS_STRING("tests.config.amconfig"))->IsValid());
            REQUIRE(fs.OpenFile(AM_OS_STRING("some_random_file.ext")) == nullptr);
        }

        SECTION("can close filesystem")
        {
            fs.StartCloseFileSystem();
            REQUIRE(fs.TryFinalizeCloseFileSystem());
        }

        SECTION("can reopen already opened filesystem")
        {
            REQUIRE(fs.TryFinalizeOpenFileSystem());

            fs.StartOpenFileSystem();
            while (!fs.TryFinalizeOpenFileSystem())
                Thread::Sleep(1);

            REQUIRE(fs.TryFinalizeOpenFileSystem());
        }
    }
}