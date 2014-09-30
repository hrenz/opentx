#include "../opentx.h"

TelemetryItem telemetryItems[TELEM_VALUES_MAX];

void TelemetryItem::setValue(int32_t newVal, uint8_t flags)
{
  if (flags == TELEM_INPUT_CELLS) {
    uint32_t data = uint32_t(newVal);
    uint8_t cellIndex = data & 0xF;
    uint8_t count = (data & 0xF0) >> 4;
    if (count != cells.count) {
      clear();
      cells.count = count;
    }
    cells.values[cellIndex].set(((data & 0x000FFF00) >>  8) / 5);
    if (cellIndex+1 < cells.count) {
      cells.values[cellIndex+1].set(((data & 0xFFF00000) >> 20) / 5);
    }
    if (cellIndex+2 >= cells.count) {
      newVal = 0;
      for (int i=0; i<count; i++) {
        if (cells.values[i].state) {
          newVal += cells.values[i].value;
        }
        else {
          // we didn't receive all cells values
          return;
        }
      }
    }
    else {
      // we didn't receive all cells values
      return;
    }
  }
  else if (flags == TELEM_INPUT_FLAGS_AUTO_OFFSET) {
    if (!isAvailable()) {
      offsetAuto = -newVal;
    }
    newVal += offsetAuto;
  }
  else if (flags == TELEM_INPUT_FLAGS_FILTERING) {
    if (!isAvailable()) {
      for (int i=0; i<TELEMETRY_AVERAGE_COUNT; i++) {
        filterValues[i] = newVal;
      }
    }
    else {
      // calculate the average from values[] and value
      // also shift readings in values [] array
      unsigned int sum = filterValues[0];
      for (int i=0; i<TELEMETRY_AVERAGE_COUNT-1; i++) {
        int32_t tmp = filterValues[i+1];
        filterValues[i] = tmp;
        sum += tmp;
      }
      filterValues[TELEMETRY_AVERAGE_COUNT-1] = newVal;
      sum += newVal;
      newVal = sum/(TELEMETRY_AVERAGE_COUNT+1);
    }
  }

  if (!isAvailable() || newVal < min) {
    min = newVal;
  }
  if (!isAvailable() || newVal > max) {
    max = newVal;
  }
  value = newVal;
  lastReceived = now();
}

bool TelemetryItem::isAvailable()
{
  return (lastReceived != TELEMETRY_VALUE_UNAVAILABLE);
}

bool TelemetryItem::isFresh()
{
  return (lastReceived < TELEMETRY_VALUE_TIMER_CYCLE) && (uint8_t(now() - lastReceived) < 2);
}

bool TelemetryItem::isOld()
{
  return (lastReceived == TELEMETRY_VALUE_OLD);
}

void TelemetryItem::eval(const TelemetrySensor & sensor)
{
  switch(sensor.formula) {
    case TELEM_FORMULA_CELL:
    {
      TelemetryItem & cellsItem = telemetryItems[sensor.cell.source-1];
      if (cellsItem.isOld()) {
        lastReceived = TELEMETRY_VALUE_OLD;
      }
      else {
        uint8_t index = sensor.cell.index;
        if (index == 0 || index == 7) {
          bool lowest = (index == 0);
          index = 0;
          for (int i=0; i<cellsItem.cells.count; i++) {
            if (cellsItem.cells.values[i].state) {
              if (index == 0 || (lowest && cellsItem.cells.values[i].value < cellsItem.cells.values[index-1].value) || (!lowest && cellsItem.cells.values[i].value > cellsItem.cells.values[index-1].value)) {
                index = i+1;
              }
            }
            else {
              index = 0;
              break;
            }
          }
        }
        index -= 1;
        if (index < cellsItem.cells.count && cellsItem.cells.values[index].state) {
          setValue(cellsItem.cells.values[index].value, sensor.inputFlags);
        }
      }
      break;
    }

    case TELEM_FORMULA_ADD:
    case TELEM_FORMULA_AVERAGE:
    {
      int32_t value=0, count=0, available=0;
      for (int i=0; i<4; i++) {
        uint8_t source = sensor.calc.sources[i];
        if (source) {
          TelemetryItem & telemetryItem = telemetryItems[source-1];
          if (sensor.formula == TELEM_FORMULA_ADD) {
            if (!telemetryItem.isAvailable()) {
              return;
            }
            else if (telemetryItem.isOld()) {
              lastReceived = TELEMETRY_VALUE_OLD;
              return;
            }
          }
          else {
            if (telemetryItem.isAvailable())
              available = 1;
            else
              continue;
            if (telemetryItem.isOld())
              continue;
            else
              count += 1;
          }
          value += telemetryItem.value;
        }
      }
      if (sensor.formula == TELEM_FORMULA_AVERAGE) {
        if (count == 0) {
          if (available)
            lastReceived = TELEMETRY_VALUE_OLD;
          return;
        }
        else {
          value = (value + count/2) / count;
        }
      }
      setValue(value, sensor.inputFlags);
      break;
    }

    default:
      break;
  }
}

int getTelemetryIndex(TelemetryProtocol protocol, uint16_t id, uint8_t instance)
{
  int available = -1;

  for (int index=0; index<TELEM_VALUES_MAX; index++) {
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];
    if (telemetrySensor.id == id && telemetrySensor.instance == instance) {
      return index;
    }
    else if (available < 0 && telemetrySensor.id == 0) {
      available = index;
    }
  }

  if (available >= 0) {
    switch (protocol) {
#if defined(FRSKY_SPORT)
      case TELEM_PROTO_FRSKY_SPORT:
        frskySportSetDefault(available, id, instance);
        break;
#endif

      default:
        break;
    }
  }

  return available;
}

void delTelemetryIndex(uint8_t index)
{
  memclear(&g_model.telemetrySensors[index], sizeof(TelemetrySensor));
  telemetryItems[index].clear();
  eeDirty(EE_MODEL);
}

int availableTelemetryIndex()
{
  for (int index=0; index<TELEM_VALUES_MAX; index++) {
    TelemetrySensor & telemetrySensor = g_model.telemetrySensors[index];
    if (telemetrySensor.id == 0) {
      return index;
    }
  }
  return -1;
}

void setTelemetryValue(TelemetryProtocol protocol, uint16_t id, uint8_t instance, int32_t value, uint32_t flags)
{
  int index = getTelemetryIndex(protocol, id, instance);

  if (index >= 0) {
    telemetryItems[index].setValue(value, flags ? flags : g_model.telemetrySensors[index].inputFlags);
  }
  else {
    // TODO error too many sensors
  }
}

void TelemetrySensor::init(const char *label, uint8_t unit, uint8_t inputFlags)
{
  memclear(this->label, TELEM_LABEL_LEN);
  strncpy(this->label, label, TELEM_LABEL_LEN);
  this->unit = unit;
  this->inputFlags = inputFlags;
}

int32_t TelemetrySensor::getValue(int32_t value, uint8_t & precision)
{
  if (type == TELEM_TYPE_CUSTOM) {
    if (custom.ratio)
      value *= custom.ratio;
    value += custom.offset;
  }
  if (prec) {
    precision = 1;
    if (prec == 2) {
      if (value >= 10000) {
        value = div10_and_round(value);
      }
      else {
        precision = 2;
      }
    }
  }
  else {
    precision = 0;
  }
  return value;
}

