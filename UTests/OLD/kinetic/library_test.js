mergeInto(LibraryManager.library, {
    $emp_info: { objs:[], images:[], image_load_count:0, image_error_count:0, callbacks:{} },

    // Below are a functions that weren't as easy to inline in the C++

    EMP_Tween_Build__deps: ['$emp_info'],
    EMP_Tween_Build: function(target_id, seconds) {
        var settings_id = emp_info.objs.length;
        emp_info.objs[settings_id] = {
            node: emp_info.objs[target_id],
            duration: seconds
        };  // Fill out configuration info.
        emp_info.objs[settings_id+1] = 0; // Preserve for tween.
        return settings_id;        
    },

    EMP_Tween_Clone__deps: ['$emp_info'],
    EMP_Tween_Clone: function(orig_settings_id) {
        var settings_id = emp_info.objs.length;

        // Copy the original settings into the new structure.
        emp_info.objs[settings_id] = {};
        for (var key in emp_info.objs[orig_settings_id]) {
            (emp_info.objs[settings_id])[ key ] = (emp_info.objs[orig_settings_id])[ key ];
        }

        emp_info.objs[settings_id+1] = 0; // Preserve for tween object.
        return settings_id;        
    },


    EMP_Rect_Build__deps: ['$emp_info'],
    EMP_Rect_Build: function(_x, _y, _w, _h, _fill, _stroke, _stroke_width, _draggable) {
        var obj_id = emp_info.objs.length;               // Determine the next free id for a Kinetic object.
        _fill = Pointer_stringify(_fill);                // Make sure string values are properly converted (colors)
        _stroke = Pointer_stringify(_stroke);
        emp_info.objs[obj_id] = new Kinetic.Rect({       // Build the new rectangle!
            x: _x,
            y: _y,
            width: _w,
            height: _h,
            fill: _fill,
            stroke: _stroke,
            strokeWidth: _stroke_width,
            draggable: _draggable
        });
        return obj_id;                                   // Return the Kinetic object id.
    },


    EMP_RegularPolygon_Build__deps: ['$emp_info'],
    EMP_RegularPolygon_Build: function(_x, _y, _sides, _radius, _fill, _stroke, _stroke_width, _draggable) {
        var obj_id = emp_info.objs.length;                   // Determine the next free id for a Kinetic object.
        _fill = Pointer_stringify(_fill);                    // Make sure string values are properly converted (colors)
        _stroke = Pointer_stringify(_stroke);
        emp_info.objs[obj_id] = new Kinetic.RegularPolygon({ // Build the new regular polygon!
                x: _x,
                y: _y,
                sides: _sides,
                radius: _radius,
                fill: _fill,
                stroke: _stroke,
                strokeWidth: _stroke_width,
                draggable: _draggable
            });
        return obj_id;                                       // Return the Kinetic object id.
    },


    EMP_Animation_Build__deps: ['$emp_info'],
    EMP_Animation_Build: function(callback_ptr, layer_id) {
        var obj_id = emp_info.objs.length;                   // Determine the next free id for a Kinetic object.
        emp_info.objs[obj_id] = new Kinetic.Animation(function(frame) {
            var ptr= Module._malloc(16); // 4 ints @ 4 bytes each...
            setValue(ptr,    frame.timeDiff,  'i32');
            setValue(ptr+4,  frame.lastTime,  'i32');
            setValue(ptr+8,  frame.time,      'i32');
            setValue(ptr+12, frame.frameRate, 'i32');

            empJSDoCallback(callback_ptr, ptr);

            Module._free(ptr);
        }, emp_info.objs[layer_id]);
        return obj_id;
    },


    EMP_Animation_Build_NoFrame__deps: ['$emp_info'],
    EMP_Animation_Build_NoFrame: function(callback_ptr, layer_id) {
        var obj_id = emp_info.objs.length;                   // Determine the next free id for a Kinetic object.
        emp_info.objs[obj_id] = new Kinetic.Animation(function(frame) {
            empJSDoCallback(callback_ptr, 0);
        }, emp_info.objs[layer_id]);
        return obj_id;
    },


    EMP_Custom_Shape_Build__deps: ['$emp_info'],
    EMP_Custom_Shape_Build: function(_x, _y, _w, _h, draw_callback) {
        var obj_id = emp_info.objs.length;                   // Determine the next free id for a Kinetic object
        emp_info.objs[obj_id] = new Kinetic.Shape({          // Build the shape!
            x: _x,
            y: _y,
            width: _w,
            height: _h,
            drawFunc: function(canvas) {                     // For an arbitrary shape, we just have a draw function
                emp_info.canvas = canvas;
                emp_info.ctx = canvas._context;              // WTF??  This should be canvas.getContext();
                empJSDoCallback(draw_callback, 0);
                emp_info.canvas = null;
                emp_info.ctx = null;
            }
        });
        return obj_id;                                       // Return the Kinetic object id.
    },
  
    my_js: function() {
        alert('hi');
    },
});
