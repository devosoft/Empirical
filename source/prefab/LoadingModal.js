// inject modal html and start it
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
    console.log('show loading modal'); 
    setTimeout(function () { 
        $('#loading_modal').modal('hide');
        },
        10000);
}
function DemoLoadingModal(){
    console.log("Reached loading modal demo trigger");
    $('#loading_modal').modal('show');
    setTimeout(CloseLoadingModal,5000);
}
function CloseLoadingModal(){
    $('#loading_modal').modal('hide');
    console.log('end of doc: hide loading modal'); 
}
console.log('in main of js');
AddLoadingModal();
