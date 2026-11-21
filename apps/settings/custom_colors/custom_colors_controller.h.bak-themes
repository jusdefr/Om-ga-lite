#ifndef OMEGA_LITE_CUSTOM_COLORS_CONTROLLER_H
#define OMEGA_LITE_CUSTOM_COLORS_CONTROLLER_H

#include <escher.h>

#include "color_editor_controller.h"
#include "color_settings.h"
#include "custom_colors_cells.h"

namespace OmegaLite {
namespace Colors {

/* "Personnaliser les couleurs": the page pushed from Settings.
 *
 * It lists the sixteen customizable roles, then three actions: restore the
 * original colors, save, cancel. Opening a role pushes the editor; edits stay
 * in memory (and in the live preview) until Save, so Cancel and Back restore
 * the colors that were in use when the page was opened. */
class CustomColorsController : public ViewController,
                               public ListViewDataSource,
                               public SelectableTableViewDataSource {
public:
  CustomColorsController(Responder * parentResponder, ColorSettings * settings);

  const char * title() override { return "Couleurs"; }
  View * view() override { return &m_selectableTableView; }
  void didBecomeFirstResponder() override;
  void viewWillAppear() override;
  bool handleEvent(Ion::Events::Event event) override;

  int numberOfRows() const override { return k_numberOfRoles + k_actionCount; }
  KDCoordinate rowHeight(int j) override;
  KDCoordinate cumulatedHeightFromIndex(int j) override;
  int indexFromCumulatedHeight(KDCoordinate offsetY) override;
  HighlightCell * reusableCell(int index, int type) override;
  int reusableCellCount(int type) override;
  int typeAtLocation(int i, int j) override;
  void willDisplayCellForIndex(HighlightCell * cell, int index) override;

private:
  enum {
    k_resetRow = k_numberOfRoles,
    k_saveRow,
    k_cancelRow,
    k_actionCount = 3,
    k_typeRole = 0,
    k_typeAction = 1
  };

  StackViewController * stackController() const {
    return static_cast<StackViewController *>(parentResponder());
  }
  void refreshRoleCells();
  void refreshActionCells();

  ColorSettings * m_settings;
  SelectableTableView m_selectableTableView;
  ColorRoleCell m_roleCells[k_numberOfRoles];
  ActionCell m_actionCells[k_actionCount];
  ColorEditorController m_editorController;
  bool m_saveFailed;
};

} // namespace Colors
} // namespace OmegaLite

#endif
