" If installed using git
set rtp+=~/.fzf
set softtabstop=4
set shiftwidth=4
set noexpandtab
call plug#begin('~/.config/nvim/plugs')

Plug 'https://github.com/easymotion/vim-easymotion'
Plug 'https://github.com/junegunn/fzf'
Plug '~/.fzf'
Plug 'https://github.com/junegunn/fzf.vim'
" Initialize plugin system
call plug#end()


set incsearch
" s{char}{char} to move to {char}{char}
nmap s <Plug>(easymotion-overwin-f)

" Move to line
map <Leader>l <Plug>(easymotion-bd-jk)
nmap <Leader>l <Plug>(easymotion-overwin-line)

" Move to word
map  <Leader>w <Plug>(easymotion-bd-w)
nmap <Leader>w <Plug>(easymotion-overwin-w)


autocmd TermOpen * set bufhidden=hide
tmap <Esc> <C-\><C-n>
autocmd! FileType fzf tnoremap <buffer> <esc> <c-c>

map <C-b> :Buffers<CR>
map <C-p> :Files<CR>
map <C-s> :Ag 
set ignorecase
