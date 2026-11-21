#ifndef OMEGA_LITE_CUSTOM_COLORS_CONTROLLER_H
#define OMEGA_LITE_CUSTOM_COLORS_CONTROLLER_H

#include <escher.h>
#include "color_settings.h"
#include "custom_colors_cells.h"

namespace OmegaLite {
namespace Colors {

class CustomColorsController : public ViewController,
                               public ListViewDataSource,
                               public SelectableTableViewDataSource {
public:
  CustomColorsController(Responder * parentResponder, ColorSettings * settings);

  const char * title() override { return "Thèmes"; }
  View * view() override { return &m_selectableTableView; }
  void didBecomeFirstResponder() override;
  void viewWillAppear() override;
  bool handleEvent(Ion::Events::Event event) override;

  int numberOfRows() const override { return k_themeCount; }
  KDCoordinate rowHeight(int j) override;
  KDCoordinate cumulatedHeightFromIndex(int j) override;
  int indexFromCumulatedHeight(KDCoordinate offsetY) override;
  HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  int typeAtLocation(int i, int j) override;
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;

private:
  enum {
    k_themeCount = 6,
    k_typeTheme = 0
  };

  StackViewController * stackController() const {
    return static_cast<StackViewController *>(parentResponder());
  }

  void applyTheme(int index);

  ColorSettings * m_settings;
  SelectableTableView m_selectableTableView;
  ActionCell m_themeCells[k_themeCount];
  bool m_saveFailed;
};

} // namespace Colors
} // namespace OmegaLite

#endif
