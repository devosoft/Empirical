#ifndef __LOAD_DATA_H__
#define __LOAD_DATA_H__

namespace D3 {
  
  class Dataset {
  protected:
    int id;
  public:
    Dataset();
    int GetID(){
      return this-> id;
    }

  };
  
  //By default, a dataset is just a pointer to the incoming data
  Dataset::Dataset(){
    this->id = EM_ASM_INT_V({return js.objects.length});
    EM_ASM({js.objects.push(emp.__incoming_data)});
  };
  
  class CSVDataset : public Dataset {
  public:
    CSVDataset(std::string location, std::string callback, bool header);

    void Parse(std::string contents, std::string accessor){
      D3_CALLBACK_FUNCTION_2_ARGS(d3.csv.parse, contents.c_str(),	\
				  accessor.c_str())
	}
    
    void ParseRows(std::string contents, std::string accessor){
      D3_CALLBACK_FUNCTION_2_ARGS(d3.csv.parseRows, contents.c_str(),	\
				  accessor.c_str())
	}
    
    //TODO Format and FormatRows
    
  };
  
  CSVDataset::CSVDataset(std::string location, std::string callback, bool header = true){

    if (header){
      EM_ASM_ARGS({
	  var arg1 = Pointer_stringify($0);
	  var in_string = Pointer_stringify($1);
	  var fn = window["d3"][in_string];
	  if (typeof fn === "function"){	
	    d3.csv(arg1, function(d){
		  emp.__incoming_data = d;
		  js.objects[$2] = d;
		  fn();
		}
	      );
	  } else {					
	    var fn = window["emp"][in_string];	
	    if (typeof fn === "function"){		
	      d3.csv(arg1, function(d){
		  emp.__incoming_data = d;
		  js.objects[$2] = d;
		  fn();
		});
	    } else {			
	      var fn = window[in_string];	
	      if (typeof fn === "function"){
		d3.csv(arg1, function(d){
		    emp.__incoming_data = d;
		    js.objects[$2] = d;
		    fn();
		  });
	      }				
	    }				
	  }					
	}, location.c_str(), callback.c_str(), this->id);

    } else {
    EM_ASM_ARGS({
	var arg1 = Pointer_stringify($0);
	var in_string = Pointer_stringify($1);
	var fn = window["d3"][in_string];
	if (typeof fn === "function"){	
	  d3.text(arg1, function(d){
	      emp.__incoming_data = d3.csv.parseRows(d);
	      js.objects[$2] = emp.__incoming_data;
	      fn();
	    });
	} else {					
	  var fn = window["emp"][in_string];	
	  if (typeof fn === "function"){		
	    d3.text(arg1, function(d){
		emp.__incoming_data = d3.csv.parseRows(d);
		js.objects[$2] = emp.__incoming_data;
		fn();
	      });
	  } else {			
	    var fn = window[in_string];	
	    if (typeof fn === "function"){
	      d3.text(arg1, function(d){
		    emp.__incoming_data = d3.csv.parseRows(d);
		    js.objects[$2] = emp.__incoming_data;
		    fn();
		});
	    }				
	  }				
	}					
      }, location.c_str(), callback.c_str(), this->id);
    }
  }


}

#endif
