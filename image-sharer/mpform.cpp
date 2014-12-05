/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to imgur.com
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010-2012 by Marius Orcsik <marius at habarnam dot ro>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "mpform.h"

// Qt includes

#include <QFile>
#include <QUrl>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDebug>

// KDE includes

#include <KRandom>


MPForm::MPForm()
{
    m_boundary  = "----------";
    m_boundary += KRandom::randomString(42 + 13).toLatin1();
}

MPForm::~MPForm()
{
}

void MPForm::reset()
{
    m_buffer.resize(0);
}

void MPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

bool MPForm::addPair(const QString& name, const QString& value, const QString& contentType)
{
    QByteArray str;
    QByteArray content_length = QByteArray::number(value.length());

    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!name.isEmpty())
    {
        str += "Content-Disposition: form-data; name=\"";
        str += name.toLatin1();
        str += "\"\r\n";
    }

    if (!contentType.isEmpty())
    {
        str += "Content-Type: " + contentType.toLatin1();
        str += "\r\n";
        str += "Mime-version: 1.0 ";
        str += "\r\n";
    }

    str += "Content-Length: ";
    str += content_length;
    str += "\r\n\r\n";
    str += value.toUtf8();

    m_buffer.append(str);
    m_buffer.append("\r\n");

    return true;
}

bool MPForm::addFile(const QString& name, const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();

    if (mime.isEmpty())
    {
        // if we ourselves can't determine the mime of the local file,
        // very unlikely the remote site will be able to identify it
        return false;
    }

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Couldn't open" << path;
        return false;
    }

    QByteArray imageData = imageFile.readAll();

    QByteArray str;
    QByteArray file_size = QByteArray::number(imageFile.size());
    imageFile.close();

    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"";
    str += name.toLatin1();
    str += "\"; ";
    str += "filename=\"";
    str += QFile::encodeName(imageFile.fileName());
    str += "\"\r\n";
    str += "Content-Length: ";
    str += file_size;
    str += "\r\n";
    str += "Content-Type: ";
    str +=  mime.toLatin1();
    str += "\r\n\r\n";

    m_buffer.append(str);
    //int oldSize = m_buffer.size();
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

bool MPForm::addFile(const QString& name, const QString& filePath, const QByteArray& fileData)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(filePath));
    QString mime  = ptr.name();

    if (mime.isEmpty())
    {
        // if we ourselves can't determine the mime of the local file,
        // very unlikely the remote site will be able to identify it
        return false;
    }

    QByteArray str;
    QByteArray file_size = QByteArray::number(fileData.size());

    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"";
    str += name.toLatin1();
    str += "\"; ";
    str += "filename=\"";
    str += QFile::encodeName(fileUrl.fileName());
    str += "\"\r\n";
    str += "Content-Length: ";
    str += file_size;
    str += "\r\n";
    str += "Content-Type: ";
    str +=  mime.toLatin1();
    str += "\r\n\r\n";

    m_buffer.append(str);
    //int oldSize = m_buffer.size();
    m_buffer.append(fileData);
    m_buffer.append("\r\n");

    return true;
}

QByteArray MPForm::contentType() const
{
    return "Content-Type: multipart/form-data; boundary=" + m_boundary;
}

QByteArray MPForm::boundary() const
{
    return m_boundary;
}

QByteArray MPForm::formData() const
{
    return m_buffer;
}
