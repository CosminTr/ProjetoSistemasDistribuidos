#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <serialization.h>


/* Serializa todas as keys presentes no array de strings keys
 * para o buffer keys_buf que será alocado dentro da função.
 * O array de keys a passar em argumento pode ser obtido através 
 * da função tree_get_keys. Para além disso, retorna o tamanho do
 * buffer alocado ou -1 em caso de erro.
 */
int keyArray_to_buffer(char **keys, char **keys_buf){

    int keys_size = strlen(*keys)+1;
    keys_buf = malloc(sizeof(char)+ keys_size);
    if (keys_buf == NULL) {
        return -1;
    }
    //copiar conteudo do array de strings para o array de bytes
    memcpy(&keys_buf, &keys, sizeof(keys_buf));

    return sizeof(keys_buf);
}

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
*/

 char** buffer_to_keyArray(char *keys_buf, int keys_buf_size){
    
} 

