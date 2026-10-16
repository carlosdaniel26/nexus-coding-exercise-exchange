use serde_json::Value;
use std::env;
use std::process;
use std::thread;
use std::time::{Duration, Instant};

fn fetch(client: &reqwest::blocking::Client, n: usize) -> bool {
    let res = match client
        .get(format!("http://127.0.0.1:3000/orders?n={n}"))
        .send()
    {
        Ok(res) if res.status() == 200 => res,
        _ => {
            eprintln!("feed unavailable at 127.0.0.1:3000");
            return false;
        }
    };

    let messages: Vec<Value> = match res.json() {
        Ok(messages) => messages,
        Err(_) => return false,
    };
    println!("-- {} messages --", messages.len());

    for message in &messages {
        if let Some(order) = message.get("New") {
            println!(
                "{} New {} {} {} x {}",
                order["id"],
                order["symbol"].as_str().unwrap_or(""),
                order["side"].as_str().unwrap_or(""),
                order["price"],
                order["quantity"]
            );
        } else if let Some(cancel) = message.get("Cancel") {
            println!("{} Cancel -> {}", cancel["id"], cancel["target_id"]);
        }
    }
    true
}

fn main() {
    let args: Vec<String> = env::args().collect();
    let n: usize = args.get(1).and_then(|a| a.parse().ok()).unwrap_or(10);
    let seconds: i64 = args.get(2).and_then(|a| a.parse().ok()).unwrap_or(0);
    let interval_ms: u64 = args.get(3).and_then(|a| a.parse().ok()).unwrap_or(200);

    let client = reqwest::blocking::Client::new();

    if seconds <= 0 {
        process::exit(if fetch(&client, n) { 0 } else { 1 });
    }

    let deadline = Instant::now() + Duration::from_secs(seconds as u64);
    while Instant::now() < deadline {
        fetch(&client, n);
        thread::sleep(Duration::from_millis(interval_ms));
    }
}
