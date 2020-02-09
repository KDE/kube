call plug#begin()
Plug 'junegunn/goyo.vim', { 'for': 'markdown' }
" Plug 'Shougo/neoinclude.vim'

Plug 'neomake/neomake'

Plug 'junegunn/fzf', { 'dir': '~/.fzf', 'do': './install --all' }
" Plug 'junegunn/fzf.vim'
Plug 'yuki-ycino/fzf-preview.vim'

Plug 'autozimu/LanguageClient-neovim', {
     \ 'branch': 'next',
     \ 'do': 'bash install.sh',
     \ }
Plug 'Shougo/deoplete.nvim', { 'do': ':UpdateRemotePlugins' }
Plug 'phpactor/phpactor' ,  {'do': 'composer install', 'for': 'php'}
Plug 'kristijanhusak/deoplete-phpactor'

Plug 'vim-airline/vim-airline'

" Git integration
Plug 'tpope/vim-fugitive'
Plug 'airblade/vim-gitgutter'
Plug 'codeindulgence/vim-tig'

Plug 'tpope/vim-commentary'
Plug 'tpope/vim-repeat'
Plug 'tpope/vim-abolish'
Plug 'tpope/vim-surround'
Plug 'tpope/vim-dispatch'

Plug 'vim-scripts/a.vim'
Plug 'mileszs/ack.vim'
Plug 'Lokaltog/vim-easymotion'
Plug 'sjl/gundo.vim'

Plug 'vhdirk/vim-cmake', { 'for': 'cmake' }

Plug 'peterhoeg/vim-qml'
Plug 'kassio/neoterm'
Plug 'keith/investigate.vim'
Plug 'vim-ruby/vim-ruby'
call plug#end()


" Use Vim settings, rather then Vi settings (much better!).
" This must be first, because it changes other options as a side effect.
set nocompatible

" == Visuals ==
"set t_Co=256
set background=dark
if &t_Co > 255 || has("gui_running")
    " colorscheme wombat256mod
    "colorscheme pablo
    " colorscheme xorium
    colorscheme monokai
endif

" Switch syntax highlighting on, when the terminal has colors
" Also switch on highlighting the last used search pattern.
if &t_Co > 2 || has("gui_running")
    syntax on
    set hlsearch
endif

" == Powerline==
" set laststatus=0 "never show statusbar
set laststatus=2 "always show statusbar
" python from powerline.vim import setup as powerline_setup
" python powerline_setup()
" python del powerline_setup

" == General ==
" se autoindent
set undofile
set undodir=~/.vim/undodir
set undolevels=1000  "maximum number of changes that can be undone
set undoreload=10000 "maximum number lines to save for undo on a buffer reload

" allow backspacing over everything in insert mode (same as bs=2)
set backspace=indent,eol,start

if has("vms")
  set nobackup		" do not keep a backup file, use versions instead
else
  set backup		" keep a backup file
endif
set history=700 " keep 700 lines of command line history
set ruler       " show the cursor position all the time
set showcmd     " display incomplete commands
set wrapscan    " wrap searches
set incsearch   " do incremental searching
set ignorecase  " ignore case while searching
set smartcase   " don't ignore case while searching when typing upper case
set smarttab    " insert tabs on the start of a line according to shiftwidth, not tabstop
set tabstop=4     " a tab is four spaces
set shiftwidth=4  " number of spaces to use for autoindenting
set shiftround    " use multiple of shiftwidth when indenting with '<' and'>'
set expandtab     " convert tabs to spaces
set relativenumber number
set directory=~/.cache/vim,/tmp    " use this directory for swap files (*~)
set backupdir=~/.cache/vim,/tmp    " use this directory for backup files (*~)
set autoread " automatically reload unchanged buffers if the file changed
set signcolumn=yes " alwas show the signcolumn (used for gitgutter etc)
set updatetime=100 " 100ms update delay for gitgutter and swapfiles
"use spellcheck (english by default)
set nospell
set complete+=kspell
map <silent> <leader>de :setlocal spell spelllang=de spelllang? <CR>
map <silent> <leader>en :setlocal spell spelllang=en spelllang? <CR>
map <silent> <leader>ns :setlocal nospell spelllang= spell? <CR>
inoremap <silent> <C-s> <C-x><C-k>

set hidden " hide instead of closing buffers, so we can switch without saving and undo lists remain
set wildignore=CMakeFiles,*~,.svn,CVS,.git,.hg,*.o,*.a,*.class,*.mo,*.la,*.so,*.obj,*.swp,*.jpg,*.png,*.xpm,*.gif,.DS_Store,*.aux,*.out,*.toc,tmp,*.scssc
set wildmenu
set wildmode=list:longest,full
if executable('/bin/zsh')
    " set shell=/bin/zsh\ -l
    set shell=/bin/fish
    :tnoremap <Esc><Esc> <C-\><C-n>
    :tnoremap <leader><leader> <C-\><C-n>
    :tnoremap <A-Esc> <Esc>

    :tnoremap <A-h> <C-\><C-n><C-w>h
    :tnoremap <A-j> <C-\><C-n><C-w>j
    :tnoremap <A-k> <C-\><C-n><C-w>k
    :tnoremap <A-l> <C-\><C-n><C-w>l
    :nnoremap <A-h> <C-w>h
    :nnoremap <A-j> <C-w>j
    :nnoremap <A-k> <C-w>k
    :nnoremap <A-l> <C-w>l
    let g:terminal_scrollback_buffer_size = 100000

    ":tnoremap <leader>a <C-\><C-n>:Ttoggle<CR>
endif

" Show trailing whitespace:
match ErrorMsg '\s\+$'

" Set leader key
let mapleader = ","

" clear search matching
noremap <leader><space> :noh<cr>:call clearmatches()<cr>

" fzf
" let $FZF_DEFAULT_COMMAND = 'ag -g ""'
let g:fzf_preview_filelist_command = 'ag -g ""'
let g:fzf_preview_command = 'bat --color=always --style=grid {-1}'
" noremap <leader>t :Files<cr>
noremap <leader>t :FzfPreviewProjectFiles<cr>
noremap <leader>b :FzfPreviewBuffers<cr>
" noremap <leader>b :Buffers<cr>

" Always search very magic (so we can do search1|search2)
" :nnoremap / /\v
" :cnoremap s/ s/\v

" Quickly edit/reload the vimrc file
nmap <silent> <leader>ev :e $MYVIMRC<CR>
nmap <silent> <leader>sv :so $MYVIMRC<CR>

" quit with leader q
nmap <silent> <leader>q :qa<CR>
" write with leader w
nmap <silent> <leader>w :w<CR>

" toggle terminal
nnoremap <silent> <leader>a :Ttoggle<CR><C-w>ja
tnoremap <silent> <leader>a <C-\><C-n>:Ttoggle<CR>
nmap <silent> <leader>m :T1 srcbuild make install<CR>

" Don't use Ex mode, use Q for formatting
map Q gq

" Paste from yankregister instead of default to support repeated replace operations
" vnoremap p "0p
vnoremap P "0p

" Don't loose selection while indenting
vnoremap < <gv
vnoremap > >gv

" allow the . to execute once for each line of a visual selection
vnoremap . :normal .<CR>

" CTRL-U in insert mode deletes a lot.  Use CTRL-G u to first break undo,
" so that you can undo CTRL-U after inserting a line break.
inoremap <C-U> <C-G>u<C-U>

" moving in buffers
nnoremap <C-L> :bn<CR>
nnoremap <C-H> :bp<CR>
nnoremap <C-x> :bd<CR>

" increment and decrement with alt
nnoremap <A-a> <C-a>
nnoremap <A-x> <C-x>

" camelcasemotion - replace default motions
" map w <Plug>CamelCaseMotion_w
" map b <Plug>CamelCaseMotion_b
" map e <Plug>CamelCaseMotion_e
" sunmap w
" sunmap b
" sunmap e

" omap iw <Plug>CamelCaseMotion_iw
" xmap iw <Plug>CamelCaseMotion_iw
" omap ib <Plug>CamelCaseMotion_ib
" xmap ib <Plug>CamelCaseMotion_ib
" omap ie <Plug>CamelCaseMotion_ie
" xmap ie <Plug>CamelCaseMotion_ie

" yankring with alt
"let g:yankring_replace_n_pkey = '<C-p>'
"let g:yankring_replace_n_nkey = '<C-n>'

nnoremap <F1> :NERDTreeToggle <CR>  
" nnoremap <F2> :FufFile <CR>
nnoremap <F3> :GitGutterToggle <CR>
nnoremap <F4> :TlistToggle <CR>
nnoremap <F5> :GundoToggle<CR>

" syntax check Bash script
map <leader>cb :!bash -n %<cr>

" build tags of your own project with Ctrl-F11
nnoremap <F11> :!ctags -R --sort=yes --c++-kinds=+p --fields=+iaS --extra=+q .<CR>

" use :w!! if you forgot to sudo open a file
cmap w!! w !sudo tee % >/dev/null

"fix commant-t abort
let g:CommandTCancelMap=['<ESC>','<C-c>']

"Esc with jk
:imap jk <Esc>

" In many terminal emulators the mouse works just fine, thus enable it.
if has('mouse')
 set mouse=a
endif


" Automatically load doxyen syntax for c/c++ and a bunch other languages
let g:load_doxygen_syntax=1

" Investiage 
let g:investigate_command_for_python = '/usr/bin/zeal ^s'

" Only do this part when compiled with support for autocommands.
if has("autocmd")
  " Enable file type detection.
  " Use the default filetype settings, so that mail gets 'tw' set to 72,
  " 'cindent' is on in C files, etc.
  " Also load indent files, to automatically do language-dependent indenting.
  filetype plugin indent on

  " Put these in an autocmd group, so that we can delete them easily.
  augroup vimrcEx
  au!

  " When editing a file, always jump to the last known cursor position.
  " Don't do it when the position is invalid or when inside an event handler
  " (happens when dropping a file on gvim).
  " Also don't do it when the mark is in the first line, that is the default
  " position when opening a file.
  autocmd BufReadPost *
    \ if line("'\"") > 1 && line("'\"") <= line("$") |
    \   exe "normal! g`\"" |
    \ endif

  augroup END

  " Reset cursor to beginning for commit messages
  au FileType gitcommit au! BufEnter COMMIT_EDITMSG call setpos('.', [0, 1, 1, 0])
else
  set autoindent		" always set autoindenting on
endif " has("autocmd")

" == Convenience Functions ==

" Convenient command to see the difference between the current buffer and the
" file it was loaded from, thus the changes you made.
command! DiffOrig vert new | set bt=nofile | r # | 0d_ | diffthis
		  \ | wincmd p | diffthis

command! Linebreak %s///g

" Clean trailing whitespace
command! Cleanspaces :%s/\s\+$//

command! Converthtml :%s/&gt;/>/ge | %s/&lt;/</ge | %s/&amp;/&/ge | %s/&quot;/"/ge

command! -range Escape <line1>,<line2>s/"/\\"/ge

command! FormatXML :%!python3 -c "import xml.dom.minidom, sys; print(xml.dom.minidom.parse(sys.stdin).toprettyxml())"
nnoremap = :FormatXML<Cr>

" function! DoCleanBrackets()
"   " if a:visual
"   "   normal! gv
"   " endif
"   '<,'>s/( /(/g | %s/ )/)/g
" endfunction

nnoremap ds<space> F<space>xf<space>x

" Clean spaces in brackets
if !exists(":Cleanbrackets")
  command Cleanbrackets :%s/( /(/g | %s/ )/)/g
  " command -range Cleanbrackets <line1>,<line2>call DoCleanBrackets()
  " if a:visual
  "   normal! gv
  " endif
  " command Cleanbrackets :'<,'>s/( /(/g | %s/ )/)/g
endif

" Paste clipboard
set clipboard=unnamedplus

command! -range Columnize <line1>,<line2>!column -t

command! -range SquashSpaces :<line1>,<line2>s/  */ /

" == Plugin Settings ==
" === Nerdtree  ===
let NERDTreeShowHidden=1

" === Taglist ===
let Tlist_WinWidth = 50

" == Commentary ==
autocmd FileType c,cpp,cs,java,qml setlocal commentstring=//\ %s


" === Rainbow parantheses ===
" au VimEnter * RainbowParenthesesToggle
" au Syntax * RainbowParenthesesLoadRound
" au Syntax * RainbowParenthesesLoadSquare
" au Syntax * RainbowParenthesesLoadBraces
" au Syntax * RainbowParenthesesLoadChevrons
"
" let g:rbpt_colorpairs = [
"     \ ['brown',       'RoyalBlue3'],
"     \ ['Darkblue',    'SeaGreen3'],
"     \ ['darkgray',    'DarkOrchid3'],
"     \ ['darkgreen',   'firebrick3'],
"     \ ['darkcyan',    'RoyalBlue3'],
"     \ ['darkred',     'SeaGreen3'],
"     \ ['darkmagenta', 'DarkOrchid3'],
"     \ ['brown',       'firebrick3'],
"     \ ['gray',        'RoyalBlue3'],
"     \ ['white',       'SeaGreen3'],
"     \ ['darkmagenta', 'DarkOrchid3'],
"     \ ['Darkblue',    'firebrick3'],
"     \ ['darkgreen',   'RoyalBlue3'],
"     \ ['darkcyan',    'SeaGreen3'],
"     \ ['darkred',     'DarkOrchid3'],
"     \ ['red',         'firebrick3'],
"     \ ]
" let g:rbpt_max = 16
" let g:rbpt_loadcmd_toggle = 0

" == Ack/Ag ==
let g:ackprg = 'ag --nogroup --nocolor --column'

" == Ultisnips ==
let g:UltiSnipsExpandTrigger="<c-j>"
let g:UltiSnipsJumpForwardTrigger="<c-j>"
let g:UltiSnipsJumpBackwardTrigger="<c-k>"

" If you want :UltiSnipsEdit to split your window.
let g:UltiSnipsEditSplit="vertical"

let g:dispatch_compilers = {
                \ 'kdesrc': 'kdesrc',
                \ 'makeobj': 'makeobj'}

function! SrcbuildNeomake(...)
    let maker = {'exe': 'srcbuild', 'args': a:000, 'errorformat': '%E%f: line %l\, col %c\, %m'}
    call neomake#Make(0, [maker])
endfunction

command! -nargs=* Srcbuild call SrcbuildNeomake(<f-args>)

" neomake_javascript_maker

" cmake filetype
au BufNewFile,BufRead CMakeLists.txt set filetype=cmake

let g:deoplete#enable_at_startup = 1
let g:deoplete#enable_refresh_always = 1
let g:deoplete#auto_complete_delay = 10
" let g:LanguageClient_serverStderr = '/tmp/clangd.stderr'
let g:LanguageClient_serverCommands = {
    \ 'cpp': ['clangd'],
    \ 'python': ['pyls'],
    \ }

"nnoremap <silent> <F2> :call LanguageClient#textDocument_rename()<CR>
nnoremap <leader>jd :call LanguageClient#textDocument_definition()<CR>
nnoremap <silent> K :call LanguageClient#textDocument_hover()<CR>
" autocmd BufRead,BufNewFile *.md :Goyo
" autocmd BufRead,BufNewFile *.c,*.cc,*.cpp,*.h,*.hh,*.hpp,CMakeLists.txt,*.py,*.qml  :Goyo!

" fromstart is the slowest, but also always results in a correct result
autocmd BufEnter * :syntax sync fromstart

autocmd FileType php setlocal commentstring=\/\/\ %s



