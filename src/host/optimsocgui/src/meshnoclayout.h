/*
 * This file is part of OpTiMSoC-GUI.
 *
 * OpTiMSoC-GUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * OpTiMSoC-GUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * (c) 2012-2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, philipp.wagner@tum.de
 */

#ifndef MESHNOCLAYOUT_H
#define MESHNOCLAYOUT_H

#include <QGraphicsScene>

#include "noclayout.h"
class MeshNoc;

class MeshNocLayout : public NocLayout
{
Q_OBJECT
public:
    explicit MeshNocLayout(MeshNoc* meshNoc, QObject *parent = 0);

    void initScene(QGraphicsScene *scene);

signals:

public slots:

private:
    MeshNoc* m_meshNoc;

};

#endif // MESHNOCLAYOUT_H
