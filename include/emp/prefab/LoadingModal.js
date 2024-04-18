/**
 * To add a loading modal to your web page while the content is rendering,
 * place this script just below the opening body tag in your HTML file:
 * <script src="https://cdn.jsdelivr.net/gh/devosoft/Empirical@master/include/emp/prefab/LoadingModal.js"></script>
 * Don't forget to close the loading modal with the emp::prefab::CloseLoadingModal() function.
 *
 *  See https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo for more details.
 *
 * @note If you modify this file, run the purge_script.sh script
 * to see your changes take effect in the browser
 */

// Loading modal html
const modal_html = '<div class="modal bd-example-modal-lg" id="loading_modal" data-backdrop="static" data-keyboard="false" tabindex="-1">\
                    <div class="modal-dialog modal-sm"> \
                      <div class="modal-content" style="width: 256px"> \
                        <div class="modal-body">\
                          <div class="spinner-border text-light" style="width: 256px; height: 256px;" role="status"> \
                          <span class="sr-only">Loading...</span> \
                        </div> \
                      </div> \
                    </div>\
                  </div>';

/// Add loading modal html to body and show modal
function AddLoadingModal() {
  document.body.innerHTML += modal_html;
  let loadingModal = document.getElementById('loading_modal');
  loadingModal.modal('show');
  setTimeout(function () {
    loadingModal.classList.remove('show');
  }, 600000); // 10 mins for testing purposes
}

/// Show loading modal for 2 seconds for demonstration
function DemoLoadingModal() {
  let loadingModal = document.getElementById('loading_modal');
  loadingModal.modal('show');
  setTimeout(CloseLoadingModal, 2000);
}

function CloseLoadingModal() {
  let loadingModal = document.getElementById('loading_modal');
  loadingModal.modal('hide');
}

// Function called when script is added to HTML
AddLoadingModal();
