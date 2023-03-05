use rayon::prelude::*;
use std::{cmp::min, collections::HashSet};

#[allow(unused)]
pub fn parallel_edit_distance(seq_one: &String, seq_two: &String) -> i32 {
    let len_seq_one = seq_one.len();
    let len_seq_two = seq_two.len();

    let seq_two = Vec::from_iter(seq_two.chars());
    let seq_one = Vec::from_iter(seq_one.chars());
    let mut alphabet = HashSet::new();
    seq_one.iter().for_each(|c| {
        alphabet.insert(c.clone());
    });
    let mut alphabet = Vec::from_iter(alphabet.into_iter());
    let mut mi = vec![vec![0_i32; len_seq_two]; alphabet.len()];
    for i in 0..alphabet.len() {
        for j in 0..len_seq_two {
            if j == 0 {
                match seq_two[j] == alphabet[i] {
                    true => mi[i][j] = 0,
                    false => mi[i][j] = -1,
                }
            } else if seq_two[j] == alphabet[i] {
                mi[i][j] = j as i32;
            } else {
                mi[i][j] = mi[i][j - 1];
            }
        }
    }
    let mut matrix = vec![vec![0_i32; len_seq_two + 1]; len_seq_one + 1];
    for i in 0..len_seq_one + 1 {
        matrix[i][0] = i as i32;
    }
    for j in 0..len_seq_two + 1 {
        matrix[0][j] = j as i32;
    }
    for i in 1..len_seq_one + 1 {
        let a = alphabet.iter().position(|x| x == &seq_one[i - 1]).unwrap();
        let mut results: Vec<i32> = (1..len_seq_two + 1)
            .into_par_iter()
            .map(|j| {
                let lmi = mi[a][j - 1] + 1;
                let lmi = lmi as i32;
                let mut re = 0;
                if (j == lmi as usize) {
                    re = matrix[i - 1][j - 1];
                } else if ((lmi == -1) || (lmi == 0)) {
                    re = min(matrix[i - 1][j - 1] + 1, matrix[i - 1][j] + 1);
                } else {
                    re = min(
                        min(matrix[i - 1][j - 1] + 1, matrix[i - 1][j] + 1),
                        matrix[i - 1][lmi as usize - 1] + (j as i32 - lmi),
                    );
                }
                re
            })
            .collect();
        results.insert(0, i as i32);
        matrix[i] = results;
    }

    return matrix[len_seq_one][len_seq_two];
}
