// To add a loading modal to your web page while the content is rendering,
// place this script just below the opening body tag in your HTML file:
// <script src="https://cdn.jsdelivr.net/gh/devosoft/Empirical@prefab/source/prefab/LoadingModal.js"></script>
// Don't forget to close the loading modal with the emp::prefab::CloseLoadingModal() function.

// See https://devosoft.github.io/empirical-prefab-demo/empirical-prefab-demo for more details.

// NOTE: If you modify this file, run the purge_script.sh script 
// to see your changes take effect in the browser

// inject loading modal html and start it
var modal_html = '<div class="modal bd-example-modal-lg" id="loading_modal" data-backdrop="static" data-keyboard="false" tabindex="-1">\
                    <div class="modal-dialog modal-sm"> \
                        <div class="modal-content" style="width: 256px"> \
                        <div class="modal-body">\
                            <div class="spinner-border text-light" style="width: 256px; height: 256px;" role="status"> \
                            <span class="sr-only">Loading...</span> \
                        </div> \
                        </div> \
                        </div>\
                        </div>';

function AddLoadingModal(){
    document.body.innerHTML += modal_html;
    $('#loading_modal').modal('show');
    setTimeout(function () { 
        $('#loading_modal').modal('hide');
        },
        10000);
}
function DemoLoadingModal(){
    $('#loading_modal').modal('show');
    setTimeout(CloseLoadingModal,2000);
}
function CloseLoadingModal(){
    $('#loading_modal').modal('hide');
}
AddLoadingModal();
