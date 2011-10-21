% This is a _very_ primitive cross-platform patch utility

% === begin Setup =====
orig_name = "octrunORIG.swg";  % The input file you want to see patched
result_name = "octrun.swg";    % The file of the patched result. Should not be equal to orig_name. 


% Unordered set of flags that will be set during a first pass of the input file
% Format: (match_string_i,flag_i)
% If any input line contains match_string_i, the flag_i flag will be set.
% Flags are set by updating a struct, so flag_i can be any datatype

flags  = {
{"{ return ptr->dims(); }", "dimref"};
{"return dim_vector(1,1);", "dimtype"}
};

% Serial stack of patches to be applied in (match_string_i, filename_i, [include_matched_i=1], [flag_matcher_i=always_true]) style.
% If flag_matcher_i, applied to the flag struct returns true, the patch will be processed. If not, the next patch in the stack is activated
% If match_string_i is found in the orig_name file, the contents of filename_i will be inserted before this line
% If include_matched_i is false, the matched line itself will not be output
% After this, i is increased by one.

isdimtype = @(flags) isfield(flags,'dimtype');
isndimtype = @(flags) ~isfield(flags,'dimtype');

isdimref = @(flags) isfield(flags,'dimref');
isndimref = @(flags) ~isfield(flags,'dimref');

patches = {
{"return dim_vector(1,1);", "octave_swig_type_dimrep.snippet",0,isdimtype}; 
{"virtual bool is_string() const", "octave_swig_type.snippet",1,isndimtype}; 
{"virtual bool is_string() const", "octave_swig_type_nodim.snippet",1,isdimtype}; 
{"virtual bool is_string() const", "octave_swig_ref.snippet",1,isndimref}; 
{"virtual bool is_string() const", "octave_swig_ref_nodim.snippet",1,isdimref}; 
};
% === End Setup ====




disp(["This is octave patcher, working on " orig_name " => " result_name])
disp("Processing flags")


orig    = fopen(orig_name,  "r");
if orig==-1
  error(["file '" orig_name "' not found in '" pwd "'"]);
end

flag_struct = struct();

orig_line = fgetl(orig);
while ischar(orig_line)
  for i=1:numel(flags)
    if index(orig_line,flags{i}{1})
      disp(["found flag '" flags{i}{1} "' => '" flags{i}{2} "'"])
      flag_struct.(flags{i}{2})=1;
    end
  end
  orig_line  = fgetl(orig);
end

fclose(orig);


disp("Processing patches")

orig    = fopen(orig_name,  "r");
if orig==-1
  error(["file '" orig_name "' not found in '" pwd "'"]);
end

patched = fopen(result_name,"w");
try

i = 1;
match_string_i = 0;

orig_line = fgetl(orig);
while ischar(orig_line)
    % Check if there are patches on the serial stack
    if match_string_i == 0
      while i <= length(patches)
        match_string_i = patches{i}{1};
        filename_i     = patches{i}{2};
        include_matched_i = 1;
        if numel(patches{i})>2
          include_matched_i = patches{i}{3};
        end
        flag_matcher_i = @(flags) 1;
        if numel(patches{i})>2
          flag_matcher_i = patches{i}{4};
        end
        if flag_matcher_i(flag_struct)
          break
        else
          i = i + 1;
        end
      end
    end
    if i <= length(patches) && index(orig_line,match_string_i) 
      disp(["found snippet '" match_string_i "' => '" filename_i "'"])
      snip = fopen(filename_i,"r");
      if snip==-1
        error(["file '" filename_i "' not found in '" pwd "'"])
      end
      patch_line = fgetl(snip);
      while ischar(patch_line)
        fdisp(patched,patch_line);
        patch_line = fgetl(snip);
      end
      fclose(snip);
      i = i + 1;
      match_string_i = 0;
      if include_matched_i
        % Copy line from original file to result file
        fdisp(patched,orig_line);
      end
    else
      % Copy line from original file to result file
      fdisp(patched,orig_line);
    end
    orig_line  = fgetl(orig);
end

fclose(orig);
fclose(patched);
disp(["Octave patcher completed succesfully."])

catch
  try
     fclose(patched);
  end_try_catch      
  unlink(result_name);  % Don't leave behind an interrupted result file
  disp(lasterr)
  disp("Octave patcher failed. Falling back to unpatched SWIG");
end_try_catch
