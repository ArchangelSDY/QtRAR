# QtRAR [![Build status](https://ci.appveyor.com/api/projects/status/v1q5u31ghi4v612e?svg=true)](https://ci.appveyor.com/project/ArchangelSDY/qtrar)

A Qt wrapper of UnRAR library.

> Disclaimer: I use this in several projects but it hasn't been tested in any serious production environment.

## Usage

```cpp
// Open a RAR archive
QtRAR archive("/path/to/archive");
if (!archive.open(QtRAR::OpenModeExtract)) {
    return;
}

// List file names
QStringList fileNames = archive.fileNameList();

// List file infos
QList<QtRARFileInfo> fileInfos = archive.fileInfoList();

// Extract one file
// QtRARFile derives from QIODevice
QtRARFile file(archive);
file.setFileName("foo/bar.txt");
if (file.open(QIODevice::ReadOnly)) {
    QByteArray content = file.readAll();
}

// QtRARFile can also be created directly
// An implicit QtRAR object will be created
QtRARFile file2("/path/to/archive", "foo/bar.txt");
```

## License

MIT
