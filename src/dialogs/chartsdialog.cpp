/***************************************************************************
 * Copyright (C) 2015-2016 by Pablo Daniel Pareja Obregon                  *
 *                                                                         *
 * This is free software; you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 2, or (at your option)     *
 * any later version.                                                      *
 *                                                                         *
 * This software is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this package; see the file COPYING.  If not, write to        *
 * the Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,   *
 * Boston, MA 02110-1301, USA.                                             *
 ***************************************************************************/

#include "chartsdialog.h"

#include "chartview.h"

#include <qwt_plot_curve.h>

namespace Caneda
{
    /*!
     * \brief Constructor.
     *
     * \param parent Parent of this object, the ChartView
     * being modified by this dialog.
     */
    ChartsDialog::ChartsDialog(ChartView *parent) : QDialog(parent)
    {
        // Initialize designer dialog
        ui.setupUi(this);

        // Set log axis checkboxes state
        ui.checkBoxXscale->setChecked(parent->isLogAxis(QwtPlot::xBottom));
        ui.checkBoxYleftScale->setChecked(parent->isLogAxis(QwtPlot::yLeft));
        ui.checkBoxYrightScale->setChecked(parent->isLogAxis(QwtPlot::yRight));
    }

    /*!
     * \brief Accept dialog
     *
     * Accept dialog and set new plot properties according to the user input.
     */
    void ChartsDialog::accept()
    {
        ChartView *parent = static_cast<ChartView*>(this->parent());

        // Set log axis checkboxes state
        parent->setLogAxis(QwtPlot::xBottom, ui.checkBoxXscale->isChecked());
        parent->setLogAxis(QwtPlot::yLeft, ui.checkBoxYleftScale->isChecked());
        parent->setLogAxis(QwtPlot::yRight, ui.checkBoxYrightScale->isChecked());

        // Accept dialog
        parent->replot();
        QDialog::accept();
    }

} // namespace Caneda
