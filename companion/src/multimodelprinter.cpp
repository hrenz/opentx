/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "helpers.h"
#include "helpers_html.h"
#include "multimodelprinter.h"
#include "appdata.h"
#include <algorithm>

MultiModelPrinter::MultiColumns::MultiColumns(int count):
  count(count),
  compareColumns(NULL)
{
  columns = new QString[count];
}

MultiModelPrinter::MultiColumns::~MultiColumns()
{
  delete[] columns;
}

void MultiModelPrinter::MultiColumns::append(const QString & str)
{
  for (int i=0; i<count; i++) {
    append(i, str);
  }
}

void MultiModelPrinter::MultiColumns::appendTitle(const QString & name)
{
  append("<b>" + name + "</b>&nbsp;");
}

void MultiModelPrinter::MultiColumns::append(int idx, const QString & str)
{
  if (compareColumns)
    compareColumns[idx].append(str);
  else
    columns[idx].append(str);
}

void MultiModelPrinter::MultiColumns::beginCompare()
{
  compareColumns = new QString[count];
}

void MultiModelPrinter::MultiColumns::endCompare(const QString & color)
{
  for (int i=0; i<count; i++) {
    QString cellColor = color;
    if (i==0 && count>1 && compareColumns[0]!=compareColumns[1])
      cellColor = "green";
    else if (i>0 && compareColumns[i]!=compareColumns[0])
      cellColor = "red";
    // font instructions must be applied at table cell level if they exist in the comparison string
    if (compareColumns[i].indexOf(QString("<td")) > -1) {
      QString t = compareColumns[i];
      int p1 = 0;
      int p2 = 0;
      int p3 = 0;
      while (t.indexOf(QString("<td"), p1) > -1) {
        p1 = t.indexOf(QString("<td"), p1);
        p2 = t.indexOf(QString(">"), p1 + 3);
        p3 = t.indexOf(QString("</td>"), p2 + 1);
        QString td = t.mid(p2 + 1, p3 - (p2 + 1));
        // remove existing font instructions from current cell
        if (td.contains(QString("<font "))) {
          int p4 = t.indexOf(QString("<font "), p2 + 1);
          int p5 = t.indexOf(QString("</font>"), p4) + 6;
          if (p5 < p3)
            t.remove(p4, p5 - p4 + 1);
        }
        t.insert(p2 + 1, QString("<font color='%1'>").arg(cellColor));
        p3 = t.indexOf(QString("</td>"), p2);
        t.insert(p3, QString("</font>"));
        p1 = p3 + QString("</font></td>").size();
      }
      compareColumns[i] = t;
      columns[i].append(QString("%1").arg(compareColumns[i]));
    }
    else {
      columns[i].append(QString("<font color='%1'>%2</font>").arg(cellColor).arg(compareColumns[i]));
    }
  }
  delete[] compareColumns;
  compareColumns = NULL;
}

template <class T>
void MultiModelPrinter::MultiColumns::append(int idx, T val)
{
  append(idx, QString("%1").arg(val));
}

QString MultiModelPrinter::MultiColumns::print()
{
  QString result = "<tr>";
  for (int i=0; i<count; i++) {
    result.append(QString("<td width='%1%'>%2</td>").arg(100.0/count).arg(columns[i]));
  }
  result.append("</tr>");
  return result;
}

bool MultiModelPrinter::MultiColumns::isEmpty()
{
  for (int i=0; i<count; i++) {
    if (!columns[i].isEmpty())
      return false;
  }
  return true;
}

void MultiModelPrinter::MultiColumns::appendLineBreak()
{
  append("<br/>");
}

void MultiModelPrinter::MultiColumns::appendSectionTableStart()
{
  append("<table cellspacing='0' cellpadding='1' width='100%' border='0' style='border-collapse:collapse'>");
}

void MultiModelPrinter::MultiColumns::appendTableEnd()
{
  append("</table>");
}

void MultiModelPrinter::MultiColumns::appendRowStart(const QString & title, const unsigned int titlewidth)
{
  append("<tr>");
  if (!(title.isEmpty()) || (titlewidth))
    appendLabelCell(QString(title), titlewidth);
}

void MultiModelPrinter::MultiColumns::appendRowEnd()
{
  append("</tr>");
}

void MultiModelPrinter::MultiColumns::appendCellStart(const unsigned int width, const bool bold)
{
  QString str = "<td";
  if (width)
    str.append(QString(" width='%1%'").arg(QString::number(width)));
  str.append(">");
  str.append(bold ? "<b>" : "");
  append(str);
}

void MultiModelPrinter::MultiColumns::appendCellEnd(const bool bold)
{
  QString str;
  str.append(bold ? "</b>" : "");
  str.append("</td>");
  append(str);
}

void MultiModelPrinter::MultiColumns::appendLabelCell(const QString & str, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableCell(str, width, align, color, true));
}

void MultiModelPrinter::MultiColumns::appendValueCell(const QString & str, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableCell(str, width, align, color, false));
}

void MultiModelPrinter::MultiColumns::appendRow(const QStringList & strl, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableRow(strl, width, align, color, false));
}

void MultiModelPrinter::MultiColumns::appendRowHeader(const QStringList & strl, const unsigned int width, const QString & align, const QString & color)
{
  append(doTableRow(strl, width, align, color, true));
}

void MultiModelPrinter::MultiColumns::appendRowBlank()
{
  append(doTableBlankRow());
}

void MultiModelPrinter::MultiColumns::appendFieldLabel(const QString & lbl)
{
  if (!lbl.isEmpty())
    appendTitle(QString("%1:").arg(lbl));
}

void MultiModelPrinter::MultiColumns::appendFieldSeparator(const bool sep)
{
  if (sep)
    append(";&nbsp;");
}

#define COMPARE(what) \
  columns.beginCompare(); \
  for (int cc=0; cc < modelPrinterMap.size(); cc++) { \
    ModelPrinter * modelPrinter = modelPrinterMap.value(cc).second; \
    const ModelData * model = modelPrinterMap.value(cc).first; \
    (void)(model); (void)(modelPrinter); \
    columns.append(cc, (what)); \
  } \
  columns.endCompare();

#define COMPARECELL(what) \
  columns.appendCellStart(); \
  COMPARE(what); \
  columns.appendCellEnd();

#define COMPARECELLWIDTH(what, width) \
  columns.appendCellStart(width); \
  COMPARE(what); \
  columns.appendCellEnd();

#define LABELCOMPARECELL(lbl, what, width) \
  columns.appendCellStart(width); \
  columns.appendFieldLabel(lbl); \
  COMPARE(what); \
  columns.appendCellEnd();

#define ROWLABELCOMPARECELL(lbl, lblw, what, width) \
  columns.appendRowStart(lbl, lblw); \
  columns.appendCellStart(width); \
  COMPARE(what); \
  columns.appendCellEnd(); \
  columns.appendRowEnd();

#define COMPARESTRING(lbl, what, sep) \
  columns.appendFieldLabel(lbl); \
  COMPARE(what); \
  columns.appendFieldSeparator(sep);

QString MultiModelPrinter::printTitle(const QString & label)
{
  return QString("<tr><td colspan='%1'><h2>").arg(modelPrinterMap.count()) + label + "</h2></td></tr>";
}

MultiModelPrinter::MultiModelPrinter(Firmware * firmware):
  firmware(firmware)
{
}

MultiModelPrinter::~MultiModelPrinter()
{
  clearModels();
}

void MultiModelPrinter::setModel(int idx, const ModelData * model, const GeneralSettings * generalSettings)
{
  if (modelPrinterMap.contains(idx) && modelPrinterMap.value(idx).second) {
    // free existing model printer
    delete modelPrinterMap.value(idx).second;
    modelPrinterMap[idx].second = NULL;
  }

  QPair<const ModelData *, ModelPrinter *> pair(model, new ModelPrinter(firmware, *generalSettings, *model));
  modelPrinterMap.insert(idx, pair);  // QMap.insert will replace any existing key
}

void MultiModelPrinter::setModel(int idx, const ModelData * model)
{
  setModel(idx, model, &defaultSettings);
}

void MultiModelPrinter::clearModels()
{
  for(int i=0; i < modelPrinterMap.size(); i++) {
    if (modelPrinterMap.value(i).second)
      delete modelPrinterMap.value(i).second;
  }
  modelPrinterMap.clear();
}

QString MultiModelPrinter::print(QTextDocument * document)
{
  if (document) document->clear();

  QString str = "<table border='1' cellspacing='0' cellpadding='3' width='100%' style='font-family: monospace;'>";
  str.append(printSetup());
  if (firmware->getCapability(Timers)) {
    str.append(printTimers());
  }
  str.append(printModules());
  if (firmware->getCapability(Heli))
    str.append(printHeliSetup());
  if (firmware->getCapability(FlightModes))
    str.append(printFlightModes());
  str.append(printInputs());
  str.append(printMixers());
  str.append(printOutputs());
  str += printCurves(document);
  if (firmware->getCapability(Gvars) && !firmware->getCapability(GvarsFlightModes))
    str.append(printGvars());
  str.append(printLogicalSwitches());
  str.append(printSpecialFunctions());
  if (firmware->getCapability(Telemetry) & TM_HASTELEMETRY) {
    str.append(printTelemetry());
    str.append(printSensors());
    if (firmware->getCapability(TelemetryCustomScreens)) {
      str.append(printTelemetryScreens());
    }
  }
  str.append("</table>");
  return str;
}

QString MultiModelPrinter::printSetup()
{
  QString str = printTitle(tr("General"));

  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  ROWLABELCOMPARECELL(tr("Name"), 25, model->name, 75);
  ROWLABELCOMPARECELL(tr("EEprom Size"), 0, modelPrinter->printEEpromSize(), 0);
  if (firmware->getCapability(ModelImage)) {
    ROWLABELCOMPARECELL(tr("Model Image"), 0, model->bitmap, 0);
  }
  ROWLABELCOMPARECELL(tr("Throttle"), 0, modelPrinter->printThrottle(), 0);
  ROWLABELCOMPARECELL(tr("Trims"), 0, modelPrinter->printSettingsTrim(), 0);
  ROWLABELCOMPARECELL(tr("Center Beep"), 0, modelPrinter->printCenterBeep(), 0);
  ROWLABELCOMPARECELL(tr("Switch Warnings"), 0, modelPrinter->printSwitchWarnings(), 0);
  if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
    ROWLABELCOMPARECELL(tr("Pot Warnings"), 0, modelPrinter->printPotWarnings(), 0);
  }
  ROWLABELCOMPARECELL(tr("Other"), 0, modelPrinter->printSettingsOther(), 0);
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printTimers()
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  columns.appendRowHeader(QStringList() << tr("Timers") << tr("Time") << tr("Switch") << tr("Countdown") << tr("Minute call") << tr("Persistence"));

  for (int i=0; i<firmware->getCapability(Timers); i++) {
    columns.appendRowStart();
    columns.appendCellStart(20, true);
    COMPARE(modelPrinter->printTimerName(i));
    columns.appendCellEnd(true);
    COMPARECELL(modelPrinter->printTimerTimeValue(model->timers[i].val));
    COMPARECELL(model->timers[i].mode.toString());
    COMPARECELL(modelPrinter->printTimerCountdownBeep(model->timers[i].countdownBeep));
    COMPARECELL(modelPrinter->printTimerMinuteBeep(model->timers[i].minuteBeep));
    COMPARECELL(modelPrinter->printTimerPersistent(model->timers[i].persistent));
    columns.appendRowEnd();
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printModules()
{
  QString str = printTitle(tr("Modules"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(NumModules); i++) {
    columns.appendRowStart();
    columns.appendCellStart(25, true);
    COMPARE(modelPrinter->printModuleType(i));
    columns.appendCellEnd(true);
    COMPARECELLWIDTH(modelPrinter->printModule(i), 75);
    columns.appendRowEnd();
  }
  if (firmware->getCapability(ModelTrainerEnable))
    columns.appendRowStart(tr("Trainer port"));
    COMPARECELL(modelPrinter->printModule(-1));
    columns.appendRowEnd();
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printHeliSetup()
{
  bool heliEnabled = false;
  for (int k=0; k < modelPrinterMap.size(); k++) {
    heliEnabled =  heliEnabled || modelPrinterMap.value(k).first->swashRingData.type != HELI_SWASH_TYPE_NONE;
  }

  if (!heliEnabled)
    return "";

  QString str = printTitle(tr("Helicopter"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  columns.appendRowStart(tr("Swash"), 20);
  LABELCOMPARECELL(tr("Type"), modelPrinter->printHeliSwashType(), 20);
  LABELCOMPARECELL(tr("Ring"), model->swashRingData.value, 20);
  columns.appendRowEnd();

  columns.appendRowBlank();
  columns.appendRowHeader(QStringList() << "" << tr("Input") << tr("Weight"));
  columns.appendRowStart(tr("Long. cyc"));
  COMPARECELL(model->swashRingData.elevatorSource.toString(model));
  COMPARECELL(model->swashRingData.elevatorWeight);
  columns.appendRowEnd();

  columns.appendRowStart(tr("Lateral cyc"));
  COMPARECELL(model->swashRingData.aileronSource.toString(model));
  COMPARECELL(model->swashRingData.aileronWeight)
  columns.appendRowEnd();

  columns.appendRowStart(tr("Collective"));
  COMPARECELL(model->swashRingData.collectiveSource.toString(model));
  COMPARECELL(model->swashRingData.collectiveWeight)
  columns.appendRowEnd();

  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printFlightModes()
{
  QString str = printTitle(tr("Flight modes"));
  // Trims
  {
    MultiColumns columns(modelPrinterMap.size());
    columns.appendSectionTableStart();
    QStringList hd = QStringList() << tr("Flight mode") << tr("Switch") << tr("Fade IN") << tr("Fade OUT");
    for (int i=0; i < getBoardCapability(getCurrentBoard(), Board::NumTrims); i++) {
      hd << RawSource(SOURCE_TYPE_TRIM, i).toString();
    }
    columns.appendRowHeader(hd);

    for (int i=0; i<firmware->getCapability(FlightModes); i++) {
      columns.appendRowStart();
      columns.appendCellStart(0,true);
      COMPARE(modelPrinter->printFlightModeName(i));
      columns.appendCellEnd(true);
      COMPARECELL(modelPrinter->printFlightModeSwitch(model->flightModeData[i].swtch));
      COMPARECELL(model->flightModeData[i].fadeIn);
      COMPARECELL(model->flightModeData[i].fadeOut);
      for (int k=0; k < getBoardCapability(getCurrentBoard(), Board::NumTrims); k++) {
        COMPARECELL(modelPrinter->printTrim(i, k));
      }
      columns.appendRowEnd();
    }

    columns.appendTableEnd();
    str.append(columns.print());
  }

  // GVars and Rotary Encoders
  int gvars = firmware->getCapability(Gvars);
  if ((gvars && firmware->getCapability(GvarsFlightModes)) || firmware->getCapability(RotaryEncoders)) {
    MultiColumns columns(modelPrinterMap.size());
    columns.appendSectionTableStart();
    QStringList hd = QStringList() << tr("Global variables");
    if (firmware->getCapability(GvarsFlightModes)) {
      for (int i=0; i<gvars; i++) {
        hd << tr("GV%1").arg(i+1);
      }
    }
    for (int i=0; i<firmware->getCapability(RotaryEncoders); i++) {
      hd << tr("RE%1").arg(i+1);
    }
    columns.appendRowHeader(hd);

    if (firmware->getCapability(GvarsFlightModes)) {
      columns.appendRowStart(tr("Name"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(model->gvarData[i].name);
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Unit"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarUnit(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Prec"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarPrec(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Min"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarMin(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Max"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarMax(i));
      }
      columns.appendRowEnd();
      columns.appendRowStart(tr("Popup"));
      for (int i=0; i<gvars; i++) {
        COMPARECELL(modelPrinter->printGlobalVarPopup(i));
      }
      columns.appendRowEnd();
    }

    columns.appendRowHeader(QStringList() << tr("Flight mode"));

    for (int i=0; i<firmware->getCapability(FlightModes); i++) {
      columns.appendRowStart();
      columns.appendCellStart(0,true);
      COMPARE(modelPrinter->printFlightModeName(i));
      columns.appendCellEnd(true);
      if (firmware->getCapability(GvarsFlightModes)) {
        for (int k=0; k<gvars; k++) {
          COMPARECELL(modelPrinter->printGlobalVar(i, k));
        }
      }
      for (int k=0; k<firmware->getCapability(RotaryEncoders); k++) {
        COMPARECELL(modelPrinter->printRotaryEncoder(i, k));
      }
      columns.appendRowEnd();
    }
    columns.appendTableEnd();
    str.append(columns.print());
  }

  return str;
}

QString MultiModelPrinter::printOutputs()
{
  QString str = printTitle(tr("Outputs"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  QStringList hd = QStringList() << tr("Channel") << tr("Subtrim") << tr("Min") << tr("Max") << tr("Direct");
  if (IS_HORUS_OR_TARANIS(firmware->getBoard()))
    hd << tr("Curve");
  if (firmware->getCapability(PPMCenter))
    hd << tr("PPM");
  if (firmware->getCapability(SYMLimits))
    hd << tr("Linear");
  columns.appendRowHeader(hd);

  for (int i=0; i<firmware->getCapability(Outputs); i++) {
    int count = 0;
    for (int k=0; k < modelPrinterMap.size(); k++)
      count = std::max(count, modelPrinterMap.value(k).first->mixes(i).size());
    if (!count)
      continue;
    columns.appendRowStart();
    columns.appendCellStart(0, true);
    COMPARE(modelPrinter->printChannelName(i));
    columns.appendCellEnd(true);
    COMPARECELL(modelPrinter->printOutputOffset(i));
    COMPARECELL(modelPrinter->printOutputMin(i));
    COMPARECELL(modelPrinter->printOutputMax(i));
    COMPARECELL(modelPrinter->printOutputRevert(i));
    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      COMPARECELL(modelPrinter->printOutputCurve(i));
    }
    if (firmware->getCapability(PPMCenter)) {
      COMPARECELL(modelPrinter->printOutputPpmCenter(i));
    }
    if (firmware->getCapability(SYMLimits)) {
      COMPARECELL(modelPrinter->printOutputSymetrical(i));
    }
    columns.appendRowEnd();
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printGvars()
{
  QString str = printTitle(tr("Global Variables"));
  int gvars = firmware->getCapability(Gvars);
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  QStringList hd;
  for (int i=0; i<gvars; i++) {
    hd << tr("GV%1").arg(i+1);
  }
  columns.appendRowHeader(hd);

  for (int i=0; i<gvars; i++) {
    COMPARECELL(model->flightModeData[0].gvars[i]);
  }
  columns.appendRowEnd();
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printInputs()
{
  QString str = printTitle(tr("Inputs"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  for (int i=0; i<std::max(4, firmware->getCapability(VirtualInputs)); i++) {
    int count = 0;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      count = std::max(count, modelPrinterMap.value(k).first->expos(i).size());
    }
    if (count > 0) {
      columns.appendRowStart();
      columns.appendCellStart(20, true);
      COMPARE(modelPrinter->printInputName(i));
      columns.appendCellEnd(true);
      columns.appendCellStart(80);
      for (int j=0; j<count; j++) {
        if (j > 0)
          columns.appendLineBreak();
        COMPARE(j<model->expos(i).size() ? modelPrinter->printInputLine(*model->expos(i)[j]) : "");
      }
      columns.appendCellEnd();
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printMixers()
{
  QString str = printTitle(tr("Mixers"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(Outputs); i++) {
    int count = 0;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      count = std::max(count, modelPrinterMap.value(k).first->mixes(i).size());
    }
    if (count > 0) {
      columns.appendRowStart();
      columns.appendCellStart(20, true);
      COMPARE(modelPrinter->printChannelName(i));
      columns.appendCellEnd(true);
      columns.appendCellStart(80);
      for (int j=0; j<count; j++) {
        if (j > 0)
          columns.appendLineBreak();
        COMPARE((j < model->mixes(i).size()) ? modelPrinter->printMixerLine(*model->mixes(i)[j], (j>0)) : "&nbsp;");
      }
      columns.appendCellEnd();
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printCurves(QTextDocument * document)
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  int count = 0;
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(NumCurves); i++) {
    bool curveEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!modelPrinterMap.value(k).first->curves[i].isEmpty()) {
        curveEmpty = false;
        break;
      }
    }
    if (!curveEmpty) {
      count++;
      columns.appendRowStart();
      columns.appendCellStart(20, true);
      COMPARE(modelPrinter->printCurveName(i));
      columns.appendCellEnd(true);
      COMPARECELL(modelPrinter->printCurve(i));
      columns.appendRowEnd();
      columns.appendRowStart("", 20);
      columns.appendCellStart();
      for (int k=0; k < modelPrinterMap.size(); k++)
        columns.append(k, QString("<br/><img src='%1' border='0' /><br/>").arg(modelPrinterMap.value(k).second->createCurveImage(i, document)));
      columns.appendCellEnd();
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Curves")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printLogicalSwitches()
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();
  int count = 0;
  for (int i=0; i<firmware->getCapability(LogicalSwitches); i++) {
    bool lsEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!modelPrinterMap.value(k).first->logicalSw[i].isEmpty()) {
        lsEmpty = false;
        break;
      }
    }
    if (!lsEmpty) {
      count++;
      columns.appendRowStart(tr("L%1").arg(i+1), 20);
      COMPARECELL(modelPrinter->printLogicalSwitchLine(i));
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Logical Switches")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printSpecialFunctions()
{
  QString str;
  MultiColumns columns(modelPrinterMap.size());
  int count = 0;
  columns.appendSectionTableStart();
  for (int i=0; i < firmware->getCapability(CustomFunctions); i++) {
    bool sfEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!modelPrinterMap.value(k).first->customFn[i].isEmpty()) {
        sfEmpty = false;
        break;
      }
    }
    if (!sfEmpty) {
      count++;
      columns.appendRowStart(tr("SF%1").arg(i+1), 20);
      COMPARECELL(modelPrinter->printCustomFunctionLine(i));
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Special Functions")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printTelemetry()
{
  QString str = printTitle(tr("Telemetry"));
  MultiColumns columns(modelPrinterMap.size());
  columns.appendSectionTableStart();

  // Analogs on non ARM boards
  if (!IS_ARM(firmware->getBoard())) {
    columns.appendRowHeader(QStringList() << tr("Analogs") << tr("Unit") << tr("Scale") << tr("Offset"));
    for (int i=0; i<2; i++) {
      columns.appendRowStart(QString("A%1").arg(i+1), 20);
      COMPARECELLWIDTH(getFrSkyUnits(model->frsky.channels[i].type), 20);
      COMPARECELLWIDTH(QString::number((model->frsky.channels[i].ratio / (model->frsky.channels[i].type==0 ? 10.0 : 1)), 10, (model->frsky.channels[i].type==0 ? 1 : 0)), 20);
      COMPARECELLWIDTH(QString::number((model->frsky.channels[i].offset*(model->frsky.channels[i].ratio / (model->frsky.channels[i].type==0 ?10.0 : 1)))/255, 10, (model->frsky.channels[i].type==0 ? 1 : 0)), 40);
      columns.appendRowEnd();
    }
  }
  else {
    // Protocol
    columns.appendRowStart(tr("Protocol"), 20);
    COMPARECELL(modelPrinter->printTelemetryProtocol(model->telemetryProtocol));
    columns.appendRowEnd();
  }

  // RSSI alarms
  {
    columns.appendRowStart(tr("RSSI Alarms"), 20);
    columns.appendCellStart(80);
    COMPARESTRING("", (IS_ARM(getCurrentBoard()) ? tr("Low") : getFrSkyAlarmType(model->rssiAlarms.level[0])), false);
    columns.append(": ");
    COMPARESTRING("", QString("&lt; %1").arg(QString::number(model->rssiAlarms.warning, 10)), true);
    COMPARESTRING("", (IS_ARM(getCurrentBoard()) ? tr("Critical") : getFrSkyAlarmType(model->rssiAlarms.level[1])), false);
    columns.append(": ");
    COMPARESTRING("", QString("&lt; %1").arg(QString::number(model->rssiAlarms.critical, 10)), true);
    COMPARESTRING(tr("Telemetry audio"), modelPrinter->printRssiAlarmsDisabled(model->rssiAlarms.disabled), false);
    columns.appendCellEnd();
    columns.appendRowEnd();
  }

  // Altimetry
  if (firmware->getCapability(HasVario)) {
    columns.appendRowStart(tr("Altimetry"), 20);
    if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
      LABELCOMPARECELL(tr("Vario source"), modelPrinter->printTelemetrySource(model->frsky.varioSource), 80);
    }
    else {
      LABELCOMPARECELL(tr("Vario source"), modelPrinter->printVarioSource(model->frsky.varioSource), 80);
    }
    columns.appendRowEnd();
    columns.appendRowStart("", 20);
    columns.appendCellStart(80);
    columns.appendTitle(tr("Vario limits >"));
    if (firmware->getCapability(HasVarioSink)) {
      COMPARESTRING(tr("Sink max"), QString::number(model->frsky.varioMin - 10), true);
      COMPARESTRING(tr("Sink min"), QString::number((float(model->frsky.varioCenterMin) / 10.0) - 0.5), true);
    }
    COMPARESTRING(tr("Climb min"), QString::number((float(model->frsky.varioCenterMax) / 10.0) + 0.5), true);
    COMPARESTRING(tr("Climb max"), QString::number(model->frsky.varioMax + 10), true);
    COMPARESTRING(tr("Center silent"), modelPrinter->printVarioCenterSilent(model->frsky.varioCenterSilent), false);
    columns.appendCellEnd();
    columns.appendRowEnd();
  }

  // Top Bar
  if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
    columns.appendRowStart(tr("Top Bar"), 20);
    columns.appendCellStart(80);
    COMPARESTRING(tr("Volts source"), modelPrinter->printTelemetrySource(model->frsky.voltsSource), true);
    COMPARESTRING(tr("Altitude source"), modelPrinter->printTelemetrySource(model->frsky.altitudeSource), false);
    columns.appendCellEnd();
    columns.appendRowEnd();
  }

  if (IS_ARM(firmware->getBoard())) {
    ROWLABELCOMPARECELL("Multi sensors", 0, modelPrinter->printIgnoreSensorIds(!model->frsky.ignoreSensorIds), 0);
  }

  // Various
  if (!IS_ARM(firmware->getBoard())) {
    columns.appendRowHeader(QStringList() << tr("Various"));
    if (!firmware->getCapability(NoTelemetryProtocol)) {
      columns.appendRowStart("", 20);
      LABELCOMPARECELL(tr("Serial protocol"), modelPrinter->printTelemetrySource(model->frsky.voltsSource), 80);
      columns.appendRowEnd();
    }
    columns.appendRowStart("", 20);
    columns.appendCellStart(80);
    QString firmware_id = g.profile[g.id()].fwType();
    if (firmware->getCapability(HasFasOffset) && firmware_id.contains("fasoffset")) {
      COMPARESTRING(tr("FAS offset"), QString("%1 A").arg(model->frsky.fasOffset/10.0), true);
    }
    if (firmware->getCapability(HasMahPersistent)) {
      COMPARESTRING(tr("mAh count"), QString("%1 mAh").arg(model->frsky.storedMah), true);
      COMPARESTRING(tr("Persistent mAh"), modelPrinter->printMahPersistent(model->frsky.mAhPersistent), false);
    }
    columns.appendRowEnd();
    columns.appendRowStart("", 20);
    columns.appendCellStart(80);
    COMPARESTRING(tr("Volts source"), modelPrinter->printVoltsSource(model->frsky.voltsSource), true);
    COMPARESTRING(tr("Current source"), modelPrinter->printCurrentSource(model->frsky.currentSource), false);
    columns.appendCellEnd();
    columns.appendRowEnd();
    columns.appendRowStart("", 20);
    LABELCOMPARECELL(tr("Blades"), model->frsky.blades, 80);
    columns.appendRowEnd();
  }
  columns.appendTableEnd();
  str.append(columns.print());
  return str;
}

QString MultiModelPrinter::printSensors()
{
  MultiColumns columns(modelPrinterMap.size());
  QString str;
  int count = 0;
  columns.appendSectionTableStart();
  columns.appendRowHeader(QStringList() << tr("Name") << tr("Type") << tr("Parameters"));
  //columns.appendRowHeader(QStringList() << tr("Name") << tr("Type") << tr("Formula/Source") << tr("Unit") << tr("Prec") << tr("Ratio") << tr("Ofst") << tr("A.Ofst") << tr("Fltr") << tr("Pers.") << tr("+ve") << tr("Log"));
  for (int i=0; i<CPN_MAX_SENSORS; ++i) {
    bool tsEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (modelPrinterMap.value(k).first->sensorData[i].isAvailable()) {
        tsEmpty = false;
        break;
      }
    }
    if (!tsEmpty) {
      count++;
      columns.appendRowStart();
      columns.appendCellStart(15, true);
      COMPARE(model->sensorData[i].nameToString(i));
      columns.appendCellEnd(true);
      COMPARECELLWIDTH(modelPrinter->printSensorTypeCond(i), 15);
      COMPARECELLWIDTH(modelPrinter->printSensorParams(i), 70);
      //COMPARESTRING("", modelPrinter->printSensorDetails(i), 0);
      columns.appendRowEnd();
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Telemetry Sensors")));
    str.append(columns.print());
  }
  return str;
}

QString MultiModelPrinter::printTelemetryScreens()
{
  MultiColumns columns(modelPrinterMap.size());
  QString str;
  int count = 0;
  columns.appendSectionTableStart();
  for (int i=0; i<firmware->getCapability(TelemetryCustomScreens); ++i) {
    bool tsEmpty = true;
    for (int k=0; k < modelPrinterMap.size(); k++) {
      if (!modelPrinterMap.value(k).first->frsky.screens[i].type == TELEMETRY_SCREEN_NONE) {
        tsEmpty = false;
        break;
      }
    }
    if (!tsEmpty) {
      count++;
      columns.appendRowStart();
      LABELCOMPARECELL(QString("%1").arg(i+1), modelPrinter->printTelemetryScreenType(model->frsky.screens[i].type), 20);
      columns.appendRowEnd();
      for (int l=0; l<4; l++) {
        COMPARE(modelPrinter->printTelemetryScreen(i, l, 80));
      }
    }
  }
  columns.appendTableEnd();
  if (count > 0) {
    str.append(printTitle(tr("Telemetry Screens")));
    str.append(columns.print());
  }
  return str;
}
